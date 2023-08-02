#include <stdlib.h>
#include "object.h"
#include <stdarg.h>
#include <stdio.h>

const struct Object NullObj = {
	.type = OBJ_NULL
};

const struct Object TrueObj = {
	.type = OBJ_BOOL,
	.value = {.boolean = true}
};

const struct Object FalseObj = {
	.type = OBJ_BOOL,
	.value = {.boolean = false }
};

struct Object nativeBoolToBoolObj(bool input) {
	if (input)
		return TrueObj;
	else
		return FalseObj;
}

bool isTruthy(struct Object obj) {
	if (obj.type == OBJ_NULL) return false;
	if (obj.type == OBJ_BOOL) return obj.value.boolean;
	if (obj.type == OBJ_INT) return obj.value.integer;
	return false;
}

char* inspectObject(const struct Object* obj) {
	char* msg = (char*)malloc(128);
	int success = 0;
	switch (obj->type) {
	case OBJ_NULL:
		success = sprintf_s(msg, 128, "NULL\n");
		break;

	case OBJ_INT:
		success = sprintf_s(msg, 128, "%lld\n", obj->value.integer);
		break;

	case OBJ_BOOL:
		success = sprintf_s(msg, 128, "%s\n", obj->value.boolean ? "true" : "false");
		break;

	case OBJ_RETURN:
		return inspectObject(obj->value.retObj);

	case OBJ_ERROR:
		success = sprintf_s(msg, 128, "ERROR: %s\n", obj->value.error.msg);
		break;
	}
	return msg;
}

const char* objectTypeToStr(const enum ObjectType type)
{
	const char* objectNames[] = {
		"NULL",
		"INTEGER",
		"BOOLEAN",
		"RETURN",
		"ERROR",
	};

	return objectNames[type];
}

const char* operatorToStr(enum OperatorType op)
{
	const char* operatorNames[] = {
	"UNKNOWN",
	"+",
	"-",
	"*",
	"/",
	">",
	">=",
	"<",
	"<=",
	"==",
	"!=",
	"!",
	"&&",
	"||",
	"%",
	};

	return operatorNames[op];
}

bool isError(struct Object obj) {
	return obj.type == OBJ_ERROR;
}

struct Object evalProgram(Program* program, struct ObjectEnvironment* env) {
	struct Object obj;

	for (size_t i = 0; i < program->size; i++) {
		obj = evalStatement(&program->statements[i], env);
		if (obj.type == OBJ_RETURN) {
			obj.type = obj.value.retObj->type;
			struct Object* trash = obj.value.retObj;
			obj.value = obj.value.retObj->value;
			free(trash);
			return obj;
		}

		if(isError(obj)) {
			return obj;
		}
	}
	return obj;
}

struct Object evalStatement(Statement* stmt, struct ObjectEnvironment* env) {
	struct Object obj = NullObj;

	switch (stmt->type) {
		case STMT_EXPR: {
			return evalExpression(stmt->expr, env);
		}

		case STMT_RETURN: {
			obj = evalExpression(stmt->expr, env);
			if(isError(obj)) {
				return obj;
			}
			struct Object* retObj = (struct Object*)malloc(sizeof * retObj);
			retObj->type = obj.type;
			retObj->value = obj.value;
			obj.type = OBJ_RETURN;
			obj.value.retObj = retObj;
			return obj;
		}

		case STMT_LET: {
			obj = evalExpression(stmt->expr, env);
			if (isError(obj)) {
				return obj;
			}
			//Now what? --> Add identifier to env
			environmentSet(env, stmt->identifier.value, obj);
			break;
		}
	}

	return NullObj;
}

struct Object evalExpression(Expression* expr, struct ObjectEnvironment* env) {
	struct Object obj = NullObj;

	switch (expr->type) {
	case EXPR_INT:
		obj.type = OBJ_INT;
		obj.value.integer = expr->integer;
		break;

	case EXPR_BOOL:
		return expr->boolean ? TrueObj : FalseObj;

	case EXPR_PREFIX:
		struct Object right = evalExpression(expr->prefix.right, env);
		if (isError(right)) {
			return right;
		}
		return evalPrefixExpression(expr->prefix.operatorType, right);

	case EXPR_INFIX:
		right = evalExpression(expr->infix.right, env);
		if (isError(right)) {
			return right;
		}

		struct Object left = evalExpression(expr->infix.left, env);
		if (isError(left)) {
			return left;
		}
		return evalInfixExpression(expr->infix.operatorType, left, right);

		case EXPR_IF:
			return evalIfExpression(expr->ifelse, env);

		case EXPR_IDENT:
		return evalIdentifier(expr, env);
	}


	return obj;
}

struct Object evalPrefixExpression(enum OperatorType op, struct Object right) {

	switch (op) {
	case OP_NEGATE:
		return evalBangOperatorExpression(right);
	case OP_SUBTRACT:
		return evalMinusPrefixExpression(right);
	default:
		return newEvalError("unknown operator: %s%s", operatorToStr(op), objectTypeToStr(right.type));
	}
}

struct Object evalBangOperatorExpression(struct Object right) {

	switch (right.type) {
	case OBJ_BOOL:
		return right.value.boolean ? FalseObj : TrueObj;

	case OBJ_NULL:
		return TrueObj;

	default:
		return FalseObj;
	}
}

struct Object evalMinusPrefixExpression(struct Object right) {
	struct Object obj = NullObj;
	if (right.type != OBJ_INT) {
		return newEvalError("unknown operator: -%s", objectTypeToStr(right.type));
	}

	obj.type = OBJ_INT;
	obj.value.integer = -right.value.integer;
	return obj;
}

struct Object evalInfixExpression(enum OperatorType op, struct Object left, struct Object right) {

	if (left.type != right.type) {
		return newEvalError("type mismatch: %s %s %s", objectTypeToStr(left.type), operatorToStr(op), objectTypeToStr(right.type));
	}

	if (left.type == OBJ_INT && right.type == OBJ_INT) {
		return evalIntegerInfixExpression(op, left, right);
	}

	if (op == OP_EQ && left.type == OBJ_BOOL && right.type == OBJ_BOOL) {
		return nativeBoolToBoolObj(left.value.boolean == right.value.boolean);
	}

	if (op == OP_NOT_EQ && left.type == OBJ_BOOL && right.type == OBJ_BOOL) {
		return nativeBoolToBoolObj(left.value.boolean != right.value.boolean);
	}

	return newEvalError("unknown operator: %s %s %s", objectTypeToStr(left.type), operatorToStr(op), objectTypeToStr(right.type));

}

struct Object evalIntegerInfixExpression(enum OperatorType op, struct Object left, struct Object right) {
	struct Object obj = NullObj;
	obj.type = OBJ_INT;

	const int64_t leftVal = left.value.integer;
	const int64_t rightVal = right.value.integer;

	switch (op) {
	case OP_ADD:
		obj.value.integer = leftVal + rightVal;
		break;

	case OP_SUBTRACT:
		obj.value.integer = leftVal - rightVal;
		break;

	case OP_MULTIPLY:
		obj.value.integer = leftVal * rightVal;
		break;

	case OP_DIVIDE:
		obj.value.integer = leftVal / rightVal;
		break;

	case OP_LT:
		return nativeBoolToBoolObj(leftVal < rightVal);

	case OP_GT:
		return nativeBoolToBoolObj(leftVal > rightVal);

	case OP_EQ:
		return nativeBoolToBoolObj(leftVal == rightVal);

	case OP_NOT_EQ:
		return nativeBoolToBoolObj(leftVal != rightVal);

	default:
		return newEvalError("unknown operator: %s %s %s", objectTypeToStr(left.type), operatorToStr(op), objectTypeToStr(right.type));
	}

	return obj;
}

struct Object evalIfExpression(struct IfExpression expr, struct ObjectEnvironment* env) {
	struct Object condition = evalExpression(expr.condition, env);
	if (isError(condition)) {
		return condition;
	}

	if (isTruthy(condition)) {
		return evalBlockStatement(expr.consequence, env);
	}
	else if (expr.alternative) {
		return evalBlockStatement(expr.alternative, env);
	}

	return NullObj;
}

struct Object evalBlockStatement(struct BlockStatement* bs, struct ObjectEnvironment* env) {
	struct Object obj;

	for (size_t i = 0; i < bs->size; i++) {
		obj = evalStatement(&bs->statements[i], env);
		if (obj.type == OBJ_RETURN || isError(obj)) {
			return obj;
		}
	}

	return obj;
}

struct Object newEvalError(const char* format, ...) {
	struct Object errorObj;
	errorObj.type = OBJ_ERROR;
	va_list argptr;
	va_start(argptr, format);
	int success = vsnprintf(errorObj.value.error.msg, 128, format, argptr);
	va_end(argptr);
	return errorObj;
}

struct Object evalIdentifier(Expression* expr, struct ObjectEnvironment* env) {
	struct Object obj = environmentGet(env, expr->ident.value);
	if(obj.type == OBJ_NULL) {
		return newEvalError("identifier not found: %s", expr->ident.value);
	}
	return obj;
}

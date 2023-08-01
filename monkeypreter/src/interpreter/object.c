#include <stdlib.h>
#include "object.h"
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
	.value =  {.boolean = false }
};

struct Object nativeBoolToBoolObj(bool input) {
	if (input)
		return TrueObj;
	else 
		return FalseObj;
}

bool isTruthy(struct Object obj) {
	if (obj.type == OBJ_NULL ) return false;
	if (obj.type == OBJ_BOOL) return obj.value.boolean;
	if (obj.type == OBJ_INT) return obj.value.integer;
	return false;
}

char* inspectObject(const struct Object* obj) {
	char* msg = (char*)malloc(128);
	int success = 0;
	switch (obj->type) {
		case OBJ_NULL:
			success = sprintf_s(msg, 128, "NULL");
			break;

		case OBJ_INT:
			success = sprintf_s(msg, 128, "%lld", obj->value.integer);
			break;

		case OBJ_BOOL:
			success = sprintf_s(msg, 128, "%s", obj->value.boolean ? "true" : "false");
			break;

		case OBJ_RETURN:
			return inspectObject(obj->value.retObj);
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
	};

	return objectNames[type];
}

struct Object evalProgram(Program* program) {
	struct Object obj;
	
	for(size_t i =0; i < program->size; i++) {
		obj = evalStatement(&program->statements[i]);
		if(obj.type == OBJ_RETURN) {
			obj.type = obj.value.retObj->type;
			struct Object* trash = obj.value.retObj;
			obj.value = obj.value.retObj->value;
			free(trash);
			return obj;
		}
	}
	return obj;
}

struct Object evalStatement(Statement* stmt) {
	switch (stmt->type) {
	case STMT_EXPR:
		return evalExpression(stmt->expr);

	case STMT_RETURN:
		struct Object obj = evalExpression(stmt->expr);
		struct Object* retObj = (struct Object*)malloc(sizeof * retObj);
		retObj->type = obj.type;
		retObj->value = obj.value;
		obj.type = OBJ_RETURN;
		obj.value.retObj = retObj;
		return obj;
	}

	return NullObj;
}

struct Object evalExpression(Expression* expr) {
	struct Object obj = NullObj;
	
	switch (expr->type) {
		case EXPR_INT:
			obj.type = OBJ_INT;
			obj.value.integer = expr->integer;
			break;

		case EXPR_BOOL:
			return expr->boolean ? TrueObj : FalseObj;

		case EXPR_PREFIX:
			struct Object right = evalExpression(expr->prefix.right);
			return evalPrefixExpression(expr->prefix.operatorType, right);
			
		case EXPR_INFIX:
			right = evalExpression(expr->infix.right);
			struct Object left = evalExpression(expr->infix.left);
			return evalInfixExpression(expr->infix.operatorType, left, right);

		case EXPR_IF:
			return evalIfExpression(expr->ifelse);
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
			return NullObj;
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
	if(right.type != OBJ_INT) {
		return obj;
	}

	obj.type = OBJ_INT;
	obj.value.integer = -right.value.integer;
	return obj;
}

struct Object evalInfixExpression(enum OperatorType op, struct Object left, struct Object right) {

	if (left.type == OBJ_INT && right.type == OBJ_INT) {
		return evalIntegerInfixExpression(op, left, right);
	}

	if(op == OP_EQ && left.type == OBJ_BOOL && right.type == OBJ_BOOL) {
		return nativeBoolToBoolObj(left.value.boolean == right.value.boolean);
	}

	if (op == OP_NOT_EQ && left.type == OBJ_BOOL && right.type == OBJ_BOOL) {
		return nativeBoolToBoolObj(left.value.boolean != right.value.boolean);
	}

	return NullObj;
	
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
		return NullObj;
	}

	return obj;
}

struct Object evalIfExpression(struct IfExpression expr) {
	struct Object condition = evalExpression(expr.condition);
	if(isTruthy(condition)) {
		return evalBlockStatement(expr.consequence);
	} else if (expr.alternative) {
		return evalBlockStatement(expr.alternative);
	} 

	return NullObj;
}

struct Object evalBlockStatement(struct BlockStatement* bs) {
	struct Object obj;

	for (size_t i = 0; i < bs->size; i++) {
		obj = evalStatement(&bs->statements[i]);
		if(obj.type == OBJ_RETURN) {
			return obj;
		}
	}

	return obj;
}
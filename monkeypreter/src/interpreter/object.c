#include "object.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "builtins.h"
#include "hash_map.h"

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

#define MAX_OBJECT_SIZE 1000000
char* inspectObject(const struct Object* obj) {
	char* msg = (char*) malloc(MAX_OBJECT_SIZE);
	msg[0] = '\0';
	int success = 0;

	if (!msg) {
		int success = fprintf(stderr, "Value of errno: %d\n", errno);
		perror("OUT OF MEMORY");
		return NULL;
	}

	switch (obj->type) {
		case OBJ_NULL:
			success = sprintf_s(msg, MAX_OBJECT_SIZE, "NULL\n");
			break;

		case OBJ_INT:
			success = sprintf_s(msg, MAX_OBJECT_SIZE, "%lld\n", obj->value.integer);
			break;

		case OBJ_BOOL:
			success = sprintf_s(msg, MAX_OBJECT_SIZE, "%s\n", obj->value.boolean ? "true" : "false");
			break;

		case OBJ_RETURN:
			return inspectObject(obj->value.retObj);

		case OBJ_ERROR:
			success = sprintf_s(msg, MAX_OBJECT_SIZE, "ERROR: %s\n", obj->value.error.msg);
			break;

		case OBJ_FUNCTION:
			strcat_s(msg, MAX_OBJECT_SIZE, "fn");
			strcat_s(msg, MAX_OBJECT_SIZE, "(");
			for (size_t i = 0; i < obj->value.function.parameters.size; i++) {
				if (i > 0) {
					strcat_s(msg, MAX_OBJECT_SIZE, ", ");
				}

				strcat_s(msg, MAX_OBJECT_SIZE, obj->value.function.parameters.values[i].value);
			}
			strcat_s(msg, MAX_OBJECT_SIZE, ") {\n");
			blockStatementToStr(msg, obj->value.function.body);
			strcat_s(msg, MAX_OBJECT_SIZE, "\n}");
			break;

		case OBJ_STRING:
			success = sprintf_s(msg, MAX_OBJECT_SIZE, "%s\n", obj->value.string);
			break;

		case OBJ_BUILTIN:
			strcat_s(msg, MAX_OBJECT_SIZE, "builtin function\n");
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
		"FUNCTION",
		"STRING",
		"BUILTIN",
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
			//obj.type = obj.value.retObj->type;
			//struct Object* trash = obj.value.retObj;
			//obj.value = obj.value.retObj->value;
			//free(trash);
			return unwrapReturnValue(obj);
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
			return obj;
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

		case EXPR_FUNCTION:
			struct Object func;
			func.type = OBJ_FUNCTION;
			func.value.function.parameters = expr->function.parameters;
			func.value.function.body = expr->function.body;
			func.value.function.env = env;
			return func;

		case EXPR_CALL:
			struct Object calledFunc = evalExpression(expr->call.function, env);
			if(isError(calledFunc)) {
				return calledFunc;
			}
			
			//Evaluate parameter expr
			struct ObjectList args = evalExpressions(expr->call.arguments, env);
			if(args.size == 1 && isError(args.objects[0])) {
				return args.objects[0];
			}

			//Apply function with evaluated args
			return applyFunction(calledFunc, args);

		case EXPR_STRING:
			obj.type = OBJ_STRING;
			strcpy_s(obj.value.string, MAX_IDENT_LENGTH, expr->string);
			break;
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

	if(left.type == OBJ_STRING && right.type == OBJ_STRING) {
		return evalStringInfixExpression(op, left, right);
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
	if(obj.type != OBJ_NULL) {
		return obj;
	}

	obj = getBuiltin(expr->ident.value);

	if(obj.type != OBJ_NULL) {
		return obj;
	}

	return newEvalError("identifier not found: %s", expr->ident.value);
}

struct ObjectList evalExpressions(struct ExpressionList expressions, struct ObjectEnvironment* env) {
	struct ObjectList args;
	args.size = 0;
	args.cap = expressions.size;
	args.objects = (struct Object*)malloc(args.cap * sizeof * args.objects);

	for(size_t i = 0; i < expressions.size; i++) {
		struct Object evaluated = evalExpression(expressions.values[i], env);
		if(isError(evaluated)) {
			args.size = 1;
			args.objects[0] = evaluated;
			return args;
		}

		if(args.size >= args.cap) {
			args.cap *= 2;
			struct Object* tmp = (struct Object*)realloc(args.objects, args.cap * sizeof * args.objects);
			if (!tmp) {
				perror("OUT OF MEMORY");
				abort();
			}
			args.objects = tmp;
		}

		args.objects[args.size] = evaluated;
		args.size++;
	}

	return args;
}

struct Object applyFunction(struct Object fn, struct ObjectList args) {

	if(fn.type == OBJ_FUNCTION) {
		struct ObjectEnvironment* extendedEnv = extendFunctionEnv(fn, args);
		struct Object evaluated = evalBlockStatement(fn.value.function.body, extendedEnv);
		//Unwrap return value to stop it from bubbling up to outer functions and stopping execution in all functions
		return unwrapReturnValue(evaluated);
	}

	if(fn.type == OBJ_BUILTIN) {
		return fn.value.builtin(args);
	}

	return newEvalError("not a function: %s", objectTypeToStr(fn.type));
}

struct ObjectEnvironment* extendFunctionEnv(struct Object fn, struct ObjectList args) {
	struct ObjectEnvironment* env = newEnclosedEnvironment(fn.value.function.env);

	for(size_t i = 0; i < fn.value.function.parameters.size; i++) {
		environmentSet(env, fn.value.function.parameters.values[i].value, args.objects[i]);
	}
	return env;
}

struct Object unwrapReturnValue(struct Object obj) {
	if(obj.type == OBJ_RETURN) {
		struct Object* trash = obj.value.retObj;
		memmove(&obj, obj.value.retObj, sizeof(struct Object));
		//obj.type = obj.value.retObj->type;
		//obj.value = obj.value.retObj->value;
		free(trash);
	}
	return obj;
}

struct Object createFunctionObject(Expression* expr, struct ObjectEnvironment* env) {
	struct Object func;
	func.type = OBJ_FUNCTION;

	//Copy over parameter data
	struct IdentifierList params;
	params.size = expr->function.parameters.size;
	params.cap = expr->function.parameters.cap;
	params.values = (Identifier*)malloc(expr->function.parameters.cap * sizeof *params.values);

	for(size_t i = 0; i < expr->function.parameters.cap; i++) {
		memcpy(&params.values[i], &expr->function.parameters.values[i], sizeof(Identifier));
	}

	func.value.function.parameters = params;

	//Copy over function body data
	struct BlockStatement* body = (struct BlockStatement*)malloc(sizeof * body);
	body->token = expr->function.body->token;
	body->size = expr->function.body->size;
	body->cap = expr->function.body->cap;

	body->statements = (Statement*)malloc(body->cap * sizeof *body->statements);

	for (size_t i = 0; i < expr->function.parameters.cap; i++) {
		memcpy(&body->statements[i], &expr->function.body->statements[i], sizeof(Statement));
	}

	func.value.function.body = body;

	//Copy over env pointer
	func.value.function.env = env;
	return func;
}

struct Object evalStringInfixExpression(enum OperatorType op, struct Object left, struct Object right) {

	if(op != OP_ADD) {
		return newEvalError("unknown operator: %s %s %s", objectTypeToStr(left.type), operatorToStr(op), objectTypeToStr(right.type));
	}

	struct Object obj;
	obj.type = OBJ_STRING;

	strcpy_s(obj.value.string, MAX_IDENT_LENGTH, left.value.string);
	strcat_s(obj.value.string, MAX_IDENT_LENGTH, right.value.string);

	return obj;	
}
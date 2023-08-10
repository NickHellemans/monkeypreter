#include "object.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "builtins.h"
#include "hash_map.h"

struct Object NullObj = {
	.type = OBJ_NULL
};

struct Object TrueObj = {
	.type = OBJ_BOOL,
	.value = {.boolean = true}
};

struct Object FalseObj = {
	.type = OBJ_BOOL,
	.value = {.boolean = false }
};

struct Object* nativeBoolToBoolObj(bool input) {
	if (input)
		return &TrueObj;
	else
		return &FalseObj;
}

struct Object* createObject(struct MonkeyGC* gc, ObjectType type) {
	struct Object* obj = (struct Object*)malloc(sizeof *obj);
	obj->type = type;
	obj->mark = false;
	obj->next = NULL;

	//Add to GC
	addToMonkeyGC(gc, obj);

	return obj;
}

void freeObject(struct Object* obj) {

	switch(obj->type) {
		case OBJ_NULL: 
		case OBJ_INT: 
		case OBJ_BOOL:
		case OBJ_STRING:
		case OBJ_ERROR: 
		case OBJ_BUILTIN: 
			//Nothing to free
			break;

		case OBJ_RETURN:
			freeObject(obj->value.retObj);
			break;

		case OBJ_FUNCTION:
			free(obj->value.function.parameters.values);
			freeBlockStatement(obj->value.function.body);
			deleteEnvironment(obj->value.function.env);
			break;

		case OBJ_ARRAY:
			for(size_t i = 0; i < obj->value.arr.size; i++) {
				if(!obj->value.arr.objects[i]->mark)
					freeObject(obj->value.arr.objects[i]);
				
			}
			free(obj->value.arr.objects);
			break;
	}

	free(obj);
}

bool isTruthy(struct Object* obj) {
	if (obj->type == OBJ_NULL) return false;
	if (obj->type == OBJ_BOOL) return obj->value.boolean;
	if (obj->type == OBJ_INT) return obj->value.integer;
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
			success = sprintf_s(msg, MAX_OBJECT_SIZE, "NULL");
			break;

		case OBJ_INT:
			success = sprintf_s(msg, MAX_OBJECT_SIZE, "%lld", obj->value.integer);
			break;

		case OBJ_BOOL:
			success = sprintf_s(msg, MAX_OBJECT_SIZE, "%s", obj->value.boolean ? "true" : "false");
			break;

		case OBJ_RETURN:
			return inspectObject(obj->value.retObj);

		case OBJ_ERROR:
			success = sprintf_s(msg, MAX_OBJECT_SIZE, "ERROR: %s", obj->value.error.msg);
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

		case OBJ_ARRAY:
			strcat_s(msg, MAX_OBJECT_SIZE, "[");
			for (size_t i = 0; i < obj->value.arr.size; i++) {
				if (i > 0) {
					strcat_s(msg, MAX_OBJECT_SIZE, ", ");
				}

				strcat_s(msg, MAX_OBJECT_SIZE, inspectObject(obj->value.arr.objects[i]));
			}

			strcat_s(msg, MAX_OBJECT_SIZE, "]");

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
		"ARRAY",
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

bool isError(struct Object* obj) {
	return obj->type == OBJ_ERROR;
}

struct Object* evalProgram(Program* program, struct ObjectEnvironment* env) {
	struct Object* obj = &NullObj;
	for (size_t i = 0; i < program->size; i++) {
		obj = evalStatement(&program->statements[i], env);
		//Mark intermediate result
		markMonkeyObject(obj);
		if (env->gc->size >= env->gc->maxSize)
			collectMonkeyGarbage(env->gc, env);

		if (obj->type == OBJ_RETURN) {
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

struct Object* evalStatement(Statement* stmt, struct ObjectEnvironment* env) {
	struct Object* obj = &NullObj;

	switch (stmt->type) {

		case STMT_EXPR: {
			return evalExpression(stmt->expr, env);
		}

		case STMT_RETURN: {
			obj = evalExpression(stmt->expr, env);
			if(isError(obj)) {
				return obj;
			}
			struct Object* retObj = createObject(env->gc, obj->type);
			retObj->value = obj->value;
			obj->type = OBJ_RETURN;
			obj->value.retObj = retObj;
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

	return obj;
}

struct Object* evalExpression(Expression* expr, struct ObjectEnvironment* env) {
	struct Object* obj = &NullObj;

	switch (expr->type) {
	case EXPR_INT:
		obj = createObject(env->gc, OBJ_INT);
		obj->value.integer = expr->integer;
		break;

	case EXPR_BOOL:
		return expr->boolean ? &TrueObj : &FalseObj;

	case EXPR_PREFIX: {
		
		struct Object* prefixRight = evalExpression(expr->prefix.right, env);
		if (isError(prefixRight)) {
			return prefixRight;
		}
		return evalPrefixExpression(expr->prefix.operatorType, prefixRight, env->gc);
	}

	case EXPR_INFIX: {
		
		struct Object* infixRight = evalExpression(expr->infix.right, env);
		if (isError(infixRight)) {
			return infixRight;
		}

		struct Object* infixLeft = evalExpression(expr->infix.left, env);
		if (isError(infixLeft)) {
			return infixLeft;
		}
		return evalInfixExpression(expr->infix.operatorType, infixLeft, infixRight, env->gc);
	}

		case EXPR_IF:
			return evalIfExpression(expr->ifelse, env);

		case EXPR_IDENT:
			return evalIdentifier(expr, env);

		case EXPR_FUNCTION: {
			struct Object* func = createObject(env->gc, OBJ_FUNCTION);
			func->value.function.parameters = expr->function.parameters;
			func->value.function.body = expr->function.body;
			func->value.function.env = env;
			return func;
		}

		case EXPR_CALL: {
			
			struct Object* calledFunc = evalExpression(expr->call.function, env);
			if(isError(calledFunc)) {
				return calledFunc;
			}
			
			//Evaluate parameter expr
			struct ObjectList args = evalExpressions(expr->call.arguments, env);
			if(args.size == 1 && isError(args.objects[0])) {
				return args.objects[0];
			}

			//Apply function with evaluated args
			return applyFunction(calledFunc, args, env->gc);
		}

		case EXPR_STRING:
			obj = createObject(env->gc, OBJ_STRING);
			strcpy_s(obj->value.string, MAX_IDENT_LENGTH, expr->string);
			break;

		case EXPR_ARRAY:
			obj = createObject(env->gc, OBJ_ARRAY);
			struct ObjectList elements = evalExpressions(expr->array.elements, env);

			if (elements.size == 1 && isError(elements.objects[0])) {
				return elements.objects[0];
			}
			obj->value.arr = elements;
			break;

		case EXPR_INDEX:
			struct Object* indexLeft = evalExpression(expr->indexExpr.left, env);
			if (isError(indexLeft)) {
				return indexLeft;
			}

			struct Object* index = evalExpression(expr->indexExpr.index, env);
			if (isError(index)) {
				return index;
			}

			return evalIndexExpression(indexLeft, index, env->gc);
	}

	return obj;
}

struct Object* evalPrefixExpression(enum OperatorType op, struct Object* right, struct MonkeyGC* gc) {

	switch (op) {
	case OP_NEGATE:
		return evalBangOperatorExpression(right);
	case OP_SUBTRACT:
		return evalMinusPrefixExpression(right, gc);
	default:
		return newEvalError(gc,"unknown operator: %s%s", operatorToStr(op), objectTypeToStr(right->type));
	}
}

struct Object* evalBangOperatorExpression(struct Object* right) {

	switch (right->type) {
	case OBJ_BOOL:
		return right->value.boolean ? &FalseObj : &TrueObj;

	case OBJ_NULL:
		return &TrueObj;

	default:
		return &FalseObj;
	}
}

struct Object* evalMinusPrefixExpression(struct Object* right, struct MonkeyGC* gc) {
	if (right->type != OBJ_INT) {
		return newEvalError(gc, "unknown operator: -%s", objectTypeToStr(right->type));
	}

	struct Object* obj = createObject(gc, OBJ_INT);
	obj->value.integer = -right->value.integer;
	return obj;
}

struct Object* evalInfixExpression(enum OperatorType op, struct Object* left, struct Object* right, struct MonkeyGC* gc) {

	if (left->type != right->type) {
		return newEvalError(gc, "type mismatch: %s %s %s", objectTypeToStr(left->type), operatorToStr(op), objectTypeToStr(right->type));
	}

	if (left->type == OBJ_INT && right->type == OBJ_INT) {
		return evalIntegerInfixExpression(op, left, right, gc);
	}

	if (op == OP_EQ && left->type == OBJ_BOOL && right->type == OBJ_BOOL) {
		return nativeBoolToBoolObj(left->value.boolean == right->value.boolean);
	}

	if (op == OP_NOT_EQ && left->type == OBJ_BOOL && right->type == OBJ_BOOL) {
		return nativeBoolToBoolObj(left->value.boolean != right->value.boolean);
	}

	if(left->type == OBJ_STRING && right->type == OBJ_STRING) {
		return evalStringInfixExpression(op, left, right, gc);
	}

	return newEvalError(gc, "unknown operator: %s %s %s", objectTypeToStr(left->type), operatorToStr(op), objectTypeToStr(right->type));

}

struct Object* evalIntegerInfixExpression(enum OperatorType op, struct Object* left, struct Object* right, struct MonkeyGC* gc) {
	struct Object* obj = createObject(gc, OBJ_INT);

	const int64_t leftVal = left->value.integer;
	const int64_t rightVal = right->value.integer;

	switch (op) {
	case OP_ADD:
		obj->value.integer = leftVal + rightVal;
		break;

	case OP_SUBTRACT:
		obj->value.integer = leftVal - rightVal;
		break;

	case OP_MULTIPLY:
		obj->value.integer = leftVal * rightVal;
		break;

	case OP_DIVIDE:
		obj->value.integer = leftVal / rightVal;
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
		return newEvalError(gc, "unknown operator: %s %s %s", objectTypeToStr(left->type), operatorToStr(op), objectTypeToStr(right->type));
	}

	return obj;
}

struct Object* evalIfExpression(struct IfExpression expr, struct ObjectEnvironment* env) {
	struct Object* condition = evalExpression(expr.condition, env);

	if (isError(condition)) {
		return condition;
	}

	if (isTruthy(condition)) {
		return evalBlockStatement(expr.consequence, env);
	}

	if (expr.alternative) {
		return evalBlockStatement(expr.alternative, env);
	}

	return &NullObj;
}

struct Object* evalBlockStatement(struct BlockStatement* bs, struct ObjectEnvironment* env) {
	struct Object* obj = &NullObj;

	for (size_t i = 0; i < bs->size; i++) {
		obj = evalStatement(&bs->statements[i], env);
		if (obj->type == OBJ_RETURN || isError(obj)) {
			return obj;
		}
	}

	return obj;
}

struct Object* newEvalError(struct MonkeyGC* gc, const char* format, ...) {
	struct Object* errorObj = createObject(gc, OBJ_ERROR);

	va_list argptr;
	va_start(argptr, format);
	int success = vsnprintf(errorObj->value.error.msg, 128, format, argptr);
	va_end(argptr);
	return errorObj;
}

struct Object* evalIdentifier(Expression* expr, struct ObjectEnvironment* env) {
	struct Object* obj = environmentGet(env, expr->ident.value);
	if(obj->type != OBJ_NULL) {
		return obj;
	}

	obj = getBuiltin(expr->ident.value);

	if(obj->type != OBJ_NULL) {
		return obj;
	}

	return newEvalError(env->gc, "identifier not found: %s", expr->ident.value);
}

struct ObjectList evalExpressions(struct ExpressionList expressions, struct ObjectEnvironment* env) {
	struct ObjectList args;
	args.size = 0;
	args.cap = expressions.size;
	args.objects = (struct Object**)malloc(args.cap * sizeof(struct Object*));

	for(size_t i = 0; i < expressions.size; i++) {
		struct Object* evaluated = evalExpression(expressions.values[i], env);
		if(isError(evaluated)) {
			args.size = 1;
			args.objects[0] = evaluated;
			return args;
		}

		if(args.size >= args.cap) {
			args.cap *= 2;
			struct Object** tmp = (struct Object**)realloc(args.objects, args.cap * sizeof(struct Object*));
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

struct Object* applyFunction(struct Object* fn, struct ObjectList args, struct MonkeyGC* gc) {

	if(fn->type == OBJ_FUNCTION) {
		struct ObjectEnvironment* extendedEnv = extendFunctionEnv(fn, args);
		struct Object* evaluated = evalBlockStatement(fn->value.function.body, extendedEnv);
		printf("Evaluated after apply func: %s\n", inspectObject(evaluated));
		//deleteEnvironment(extendedEnv);
		//Unwrap return value to stop it from bubbling up to outer functions and stopping execution in all functions
		return unwrapReturnValue(evaluated);
	}

	if(fn->type == OBJ_BUILTIN) {
		return fn->value.builtin(args, gc);
	}

	return newEvalError(gc, "not a function: %s", objectTypeToStr(fn->type));
}

struct ObjectEnvironment* extendFunctionEnv(struct Object* fn, struct ObjectList args) {
	struct ObjectEnvironment* env = newEnclosedEnvironment(fn->value.function.env);

	for(size_t i = 0; i < fn->value.function.parameters.size; i++) {
		environmentSet(env, fn->value.function.parameters.values[i].value, args.objects[i]);
	}
	return env;
}

struct Object* unwrapReturnValue(struct Object* obj) {
	
	if(obj->type == OBJ_RETURN) {
		//struct Object* trash = obj->value.retObj;
		//struct Object* retObj = (struct Object*) malloc(sizeof * retObj);
		//memmove(obj, obj->value.retObj, sizeof(struct Object));
		//obj.type = obj.value.retObj->type;
		//obj.value = obj.value.retObj->value;
		//free(trash);
		return obj->value.retObj;
	}
	return obj;
}

struct Object* evalStringInfixExpression(enum OperatorType op, struct Object* left, struct Object* right, struct MonkeyGC* gc) {

	if(op != OP_ADD) {
		return newEvalError(gc, "unknown operator: %s %s %s", objectTypeToStr(left->type), operatorToStr(op), objectTypeToStr(right->type));
	}

	struct Object* obj = createObject(gc, OBJ_STRING);

	strcpy_s(obj->value.string, MAX_IDENT_LENGTH, left->value.string);
	strcat_s(obj->value.string, MAX_IDENT_LENGTH, right->value.string);

	return obj;	
}

struct Object* evalIndexExpression(struct Object* left, struct Object* index, struct MonkeyGC* gc) {
	if(left->type == OBJ_ARRAY && index->type == OBJ_INT) {
		return evalArrayIndexExpression(left, index);
	}

	return newEvalError(gc, "index operator not supported: %s", objectTypeToStr(left->type));
}

struct Object* evalArrayIndexExpression(struct Object* arr, struct Object* index) {
	int64_t idx = index->value.integer;
	size_t max = arr->value.arr.size - 1;

	if(idx < 0 || (size_t)idx > max) {
		return &NullObj;
	}

	return arr->value.arr.objects[idx];
}
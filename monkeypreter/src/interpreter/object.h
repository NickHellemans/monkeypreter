#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "environment.h"
#include "../parser/ast.h"

typedef enum ObjectType {
	OBJ_NULL,
	OBJ_INT,
	OBJ_BOOL,
	OBJ_RETURN,
	OBJ_ERROR,
	OBJ_FUNCTION,
	OBJ_STRING,
} ObjectType;

struct ErrorObject {
	char msg[128];
};

struct FunctionObject {
	struct IdentifierList parameters;
	struct BlockStatement* body;
	struct ObjectEnvironment* env;
};

union ObjectVal {
	bool boolean;
	int64_t integer;
	char string[MAX_IDENT_LENGTH];
	struct Object* retObj;
	struct ErrorObject error;
	struct FunctionObject function;
};

struct Object {
	ObjectType type;
	union ObjectVal value;
};

struct ObjectList {
	size_t size;
	size_t cap;
	struct Object* objects;
};

char* inspectObject(const struct Object* obj);
const char* objectTypeToStr(const enum ObjectType type);
struct Object evalProgram(Program* program, struct ObjectEnvironment* env);
struct Object evalStatement(Statement* stmt, struct ObjectEnvironment* env);
struct Object evalExpression(Expression* expr, struct ObjectEnvironment* env);
struct Object evalPrefixExpression(enum OperatorType op, struct Object right);
struct Object evalBangOperatorExpression(struct Object right);
struct Object evalMinusPrefixExpression(struct Object right);
struct Object evalInfixExpression(enum OperatorType op, struct Object left, struct Object right);
struct Object evalIntegerInfixExpression(enum OperatorType op, struct Object left, struct Object right);
struct Object evalIfExpression(struct IfExpression expr, struct ObjectEnvironment* env);
struct Object evalBlockStatement(struct BlockStatement* bs, struct ObjectEnvironment* env);
struct Object newEvalError(const char* format, ...);
struct Object evalIdentifier(Expression* expr, struct ObjectEnvironment* env);
struct ObjectList evalExpressions(struct ExpressionList expressions, struct ObjectEnvironment* env);
struct Object applyFunction(struct Object fn, struct ObjectList args);
struct ObjectEnvironment* extendFunctionEnv(struct Object fn, struct ObjectList args);
struct Object unwrapReturnValue(struct Object obj);
struct Object createFunctionObject(Expression* expr, struct ObjectEnvironment* env);
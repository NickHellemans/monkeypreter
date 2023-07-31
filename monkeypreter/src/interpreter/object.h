#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "../parser/ast.h"

typedef enum ObjectType {
	OBJ_NULL,
	OBJ_INT,
	OBJ_BOOL,
} ObjectType;

union ObjectVal {
	bool boolean;
	int64_t integer;
};

struct Object {
	ObjectType type;
	union ObjectVal value;
};

char* inspectObject(const struct Object* obj);
const char* objectTypeToStr(const enum ObjectType type);
struct Object evalProgram(Program* program);
struct Object evalStatement(Statement* stmt);
struct Object evalExpression(Expression* expr);
struct Object evalPrefixExpression(enum OperatorType op, struct Object right);
struct Object evalBangOperatorExpression(struct Object right);
struct Object evalMinusPrefixExpression(struct Object right);
struct Object evalInfixExpression(enum OperatorType op, struct Object left, struct Object right);
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
	}
	return msg;
}

const char* objectTypeToStr(const enum ObjectType type)
{
	const char* objectNames[] = {
		"NULL",
		"INTEGER",
		"BOOLEAN",
	};

	return objectNames[type];
}

struct Object evalProgram(Program* program) {
	struct Object obj;
	
	for(size_t i =0; i < program->size; i++) {
		obj = evalStatement(&program->statements[i]);
	}

	return obj;
}

struct Object evalStatement(Statement* stmt) {
	switch (stmt->type) {
	case STMT_EXPR:
		return evalExpression(stmt->expr);
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
	}

	return obj;
}

struct Object evalPrefixExpression(enum OperatorType op, struct Object right) {
	
	switch (op) {
		case OP_NEGATE:
			return evalBangOperatorExpression(right);
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
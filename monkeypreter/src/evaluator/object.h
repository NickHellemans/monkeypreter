#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "environment.h"
#include "../parser/ast.h"

enum ObjectType {
	OBJ_NULL,
	OBJ_INT,
	OBJ_BOOL,
	OBJ_RETURN,
	OBJ_ERROR,
	OBJ_FUNCTION,
	OBJ_STRING,
	OBJ_BUILTIN,
	OBJ_ARRAY,
} ObjectType;

struct ErrorObject {
	char msg[128];
};

struct FunctionObject {
	struct IdentifierList parameters;
	struct BlockStatement* body;
	struct ObjectEnvironment* env;
};

struct ObjectList {
	size_t size;
	size_t cap;
	struct Object** objects;
};

union ObjectVal {
	bool boolean;
	int64_t integer;
	char string[MAX_IDENT_LENGTH];
	struct Object* retObj;
	struct ErrorObject error;
	struct FunctionObject function;
	//Builtin fn pointer that returns object
	struct Object* (*builtin) (struct ObjectList args, struct MonkeyGC* gc);
	//Array
	struct ObjectList arr;
};

struct Object {
	enum ObjectType type;
	union ObjectVal value;

	//For GC
	bool mark;
	struct Object* next;
};

extern struct Object NullObj;

extern struct Object TrueObj;

extern struct Object FalseObj;

struct Object* createObject(struct MonkeyGC* garbageCollector, enum ObjectType type);
void freeObject(struct Object* obj);
char* inspectObject(const struct Object* obj);
const char* objectTypeToStr(const enum ObjectType type);
struct Object* nativeBoolToBoolObj(bool input);
bool isTruthy(struct Object* obj);
bool isError(struct Object* obj);
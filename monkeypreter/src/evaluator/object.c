#include "object.h"
#include <stdio.h>
#include <string.h>
#include "gc.h"

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

struct Object* createObject(struct MonkeyGC* gc, enum ObjectType type) {
	struct Object* obj = (struct Object*)malloc(sizeof *obj);

	if(!obj) {
		perror("malloc (create object) returned `NULL`\n");
		exit(EXIT_FAILURE);
	}
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
			if(!obj->value.retObj->mark)
				freeObject(obj->value.retObj);
			break;

		case OBJ_FUNCTION:
			free(obj->value.function.parameters.values);
			freeBlockStatement(obj->value.function.body);
			//Do not delete env
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
	if (!msg) {
		perror("malloc (inspect object) returned `NULL`\n");
		return NULL;
	}
	msg[0] = '\0';
	int success = 0;

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

bool isError(struct Object* obj) {
	return obj->type == OBJ_ERROR;
}

#pragma once
#include "object.h"
#include <string.h>


inline struct Object* len(struct ObjectList args) {
	if(args.size != 1) {
		return newEvalError("wrong number of arguments. got=%d, want=1", args.size);
	}

	struct Object* arg = args.objects[0];

	struct Object* obj;
	if(arg->type == OBJ_STRING) {
		obj = createObject(OBJ_INT);
		obj->value.integer = (int64_t) strlen(arg->value.string);
		return obj;
	}

	if(arg->type == OBJ_ARRAY) {
		obj = createObject(OBJ_INT);
		obj->value.integer = (int64_t) arg->value.arr.size;
		return obj;
	}

	return newEvalError("argument to `len` not supported, got %s", objectTypeToStr(arg->type));
}

inline struct Object* first(struct ObjectList args) {
	if (args.size != 1) {
		return newEvalError("wrong number of arguments. got=%d, want=1", args.size);
	}

	struct Object* arg = args.objects[0];

	if(arg->type != OBJ_ARRAY) {
		return newEvalError("argument to `first` must be ARRAY, got %s", objectTypeToStr(arg->type));
	}

	if(arg->value.arr.size > 0) {
		return arg->value.arr.objects[0];
	}

	//struct Object obj;
	//obj.type = OBJ_NULL;
	return &NullObj;
}

inline struct Object* last(struct ObjectList args) {
	const size_t argSize = args.size;
	if (argSize != 1) {
		return newEvalError("wrong number of arguments. got=%d, want=1", argSize);
	}

	struct Object* arg = args.objects[0];

	if (arg->type != OBJ_ARRAY) {
		return newEvalError("argument to `last` must be ARRAY, got %s", objectTypeToStr(arg->type));
	}

	const size_t arrSize = arg->value.arr.size;

	if (arrSize > 0) {
		return arg->value.arr.objects[arrSize - 1];
	}

	//struct Object obj;
	//obj.type = OBJ_NULL;
	return &NullObj;
}

inline struct Object* cdr(struct ObjectList args) {
	const size_t argSize = args.size;
	if (argSize != 1) {
		return newEvalError("wrong number of arguments. got=%d, want=1", argSize);
	}

	struct Object* arg = args.objects[0];

	if (arg->type != OBJ_ARRAY) {
		return newEvalError("argument to `cdr` must be ARRAY, got %s", objectTypeToStr(arg->type));
	}

	const size_t arrSize = arg->value.arr.size;
	const size_t arrCap = arg->value.arr.cap;

	//if (arrSize == 1) {
	//	obj.type = OBJ_ARRAY;
	//	struct ObjectList copyList;
	//	copyList.size = 0;
	//	copyList.cap = arrCap;
	//	copyList.objects = NULL;
	//	obj.value.arr = copyList;
	//	return obj;
	//}

	if(arrSize > 0) {
		
		struct Object* obj = createObject(OBJ_ARRAY);

		struct ObjectList copyList;
		copyList.size = arrSize - 1;
		copyList.cap = arrCap;
		copyList.objects = (struct Object**)malloc(arrCap * sizeof(struct Object*));
		memcpy(copyList.objects, arg->value.arr.objects + 1, copyList.size * sizeof(struct Object*));

		obj->value.arr = copyList;
		return obj;
	}

	//obj.type = OBJ_NULL;

	return &NullObj;
}

inline struct Object* push(struct ObjectList args) {
	const size_t argSize = args.size;
	if (argSize != 2) {
		return newEvalError("wrong number of arguments. got=%d, want=2", argSize);
	}

	if(args.objects[0]->type != OBJ_ARRAY) {
		return newEvalError("argument to `push` must be ARRAY, got %s", objectTypeToStr(args.objects[0]->type));
	}
	struct Object* arr = args.objects[0];
	struct Object* objToAdd = args.objects[1];

	const size_t arrSize = arr->value.arr.size;
	const size_t arrCap = arr->value.arr.cap;

	struct Object* obj = createObject(OBJ_ARRAY);

	struct ObjectList copyList;
	copyList.size = arrSize;
	copyList.cap = copyList.size  + 1 >= arrCap ? arrCap + 1 : arrCap;
	copyList.objects = (struct Object**)malloc(copyList.cap * sizeof(struct Object*));
	memcpy(copyList.objects, arr->value.arr.objects, arrSize * sizeof(struct Object*));
	copyList.objects[copyList.size] = objToAdd;
	copyList.size++;

	obj->value.arr = copyList;

	return obj;
}

struct BuiltinFunction {
	char name[MAX_IDENT_LENGTH];
	struct Object* (*builtin) (struct ObjectList args);
};

inline struct Object* print(struct ObjectList args) {

	for(size_t i = 0; i < args.size; i++) {
		printf("%s", inspectObject(args.objects[i]));
	}

	return &NullObj;
}

struct BuiltinFunction builtinFunctions[] = {
	{"len", len},
	{"first", first},
	{"last", last},
	{"cdr", cdr},
	{"push", push},
	{"print", print},
};

struct Object builtinFunctionsObjects[] = {
	{OBJ_BUILTIN, {.builtin = len}},
	{OBJ_BUILTIN, {.builtin = first}},
	{OBJ_BUILTIN, {.builtin = last}},
	{OBJ_BUILTIN, {.builtin = cdr}},
	{OBJ_BUILTIN, {.builtin = push}},
	{OBJ_BUILTIN, {.builtin = print}},
};

#define builtinSize (sizeof(builtinFunctions) / sizeof(builtinFunctions[0]))

inline struct Object* getBuiltin(char* key) {
	struct Object* obj = &NullObj;
	for(size_t i = 0; i < builtinSize; i++) {
		if(strcmp(key, builtinFunctions[i].name) == 0) {
			//obj = createObject(OBJ_BUILTIN);
			//obj->value.builtin = builtinFunctions[i].builtin;
			//return obj;
			return &builtinFunctionsObjects[i];
		}
	}
	return obj;
}

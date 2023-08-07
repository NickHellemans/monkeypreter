#pragma once
#include "object.h"
#include <string.h>


inline struct Object len(struct ObjectList args) {
	if(args.size != 1) {
		return newEvalError("wrong number of arguments. got=%d, want=1", args.size);
	}

	struct Object arg = args.objects[0];

	if(arg.type == OBJ_STRING) {
		struct Object obj;
		obj.type = OBJ_INT;
		obj.value.integer = (int64_t) strlen(arg.value.string);
		return obj;
	}

	return newEvalError("argument to `len` not supported, got %s", objectTypeToStr(arg.type));
}

struct BuiltinFunction {
	char name[MAX_IDENT_LENGTH];
	struct Object (*builtin) (struct ObjectList args);
};

struct BuiltinFunction builtinFunctions[] = {
	{"len", len},
};

#define builtinSize (sizeof(builtinFunctions) / sizeof(builtinFunctions[0]))

inline struct Object getBuiltin(char* key) {
	struct Object obj;

	for(size_t i = 0; i < builtinSize; i++) {
		if(strcmp(key, builtinFunctions[i].name) == 0) {
			obj.type = OBJ_BUILTIN;
			obj.value.builtin = builtinFunctions[i].builtin;
			return obj;
		}
	}

	obj.type = OBJ_NULL;

	return obj;
}

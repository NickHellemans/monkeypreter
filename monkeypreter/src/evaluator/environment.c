#include "environment.h"
#include <stdio.h>
#include "hash_map.h"

struct ObjectEnvironment* newEnvironment(struct MonkeyGC* gc) {
	struct ObjectEnvironment* env = (struct ObjectEnvironment*)malloc(sizeof * env);
	if (!env) {
		perror("malloc (create env) returned `NULL`\n");
		exit(EXIT_FAILURE);
	}
	env->store = createHashMap(17);
	env->outer = NULL;
	env->gc = gc;
	return env;
}

struct ObjectEnvironment* newEnclosedEnvironment(struct ObjectEnvironment* outer) {
	struct ObjectEnvironment* env = newEnvironment(outer->gc);
	env->outer = outer;
	return env;
}

struct Object* environmentGet(struct ObjectEnvironment* env, char* key) {
	struct Object* obj = (struct Object*)lookupKeyInHashMap(env->store, key);
	if(obj == NULL && env->outer != NULL) {
		obj = environmentGet(env->outer, key);
	}

	if(obj == NULL) {
		obj = &NullObj;
	}

	return obj;
}

struct Object* environmentSet(struct ObjectEnvironment* env, char* key, struct Object* data) {
	insertIntoHashMap(env->store, key, data);
	return data;
}

void deleteEnvironment(struct ObjectEnvironment* env) {
	destroyHashMap(env->store);
	free(env);
}

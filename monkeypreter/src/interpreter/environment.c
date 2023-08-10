#include "environment.h"

#include <stdio.h>

#include "hash_map.h"

struct ObjectEnvironment* newEnvironment(struct MonkeyGC* gc) {
	struct ObjectEnvironment* env = (struct ObjectEnvironment*)malloc(sizeof * env);
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
	//printf("Added `%s` to env\n", key);
	insertIntoHashMap(env->store, key, data);
	return data;
}

void deleteEnvironment(struct ObjectEnvironment* env) {
	destroyHashMap(env->store);
	free(env);
}

void deleteAllEnvironment(struct ObjectEnvironment* env) {
	destroyHashMap(env->store);
	struct ObjectEnvironment* currEnv = env->outer;
	while (currEnv != NULL) {
		struct ObjectEnvironment* trash = currEnv;
		currEnv = currEnv->outer;
		destroyHashMap(trash->store);
		free(trash);
	}
	free(env);
}
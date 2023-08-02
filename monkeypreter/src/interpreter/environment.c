#include "environment.h"
#include "hash_map.h"

struct ObjectEnvironment newEnvironment(void) {
	struct ObjectEnvironment env;
	env.store = createHashMap(17);
	return env;
}

struct Object environmentGet(struct ObjectEnvironment* env, char* key) {
	return lookupKeyInHashMap(env->store, key);
}

struct Object environmentSet(struct ObjectEnvironment* env, char* key, struct Object data) {
	insertIntoHashMap(env->store, key, data);
	return data;
}

void deleteEnvironment(struct ObjectEnvironment* env) {
	destroyHashMap(env->store);
}
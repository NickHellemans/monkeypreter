#pragma once

struct ObjectEnvironment {
	struct HashMap* store;
};

struct ObjectEnvironment newEnvironment(void);
struct Object environmentGet(struct ObjectEnvironment* env, char* key);
struct Object environmentSet(struct ObjectEnvironment* env, char* key, struct Object data);
void deleteEnvironment(struct ObjectEnvironment* env);
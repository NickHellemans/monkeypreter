#pragma once

struct ObjectEnvironment {
	struct HashMap* store;
	struct ObjectEnvironment* outer;
};

struct ObjectEnvironment* newEnvironment(void);
struct ObjectEnvironment* newEnclosedEnvironment(struct ObjectEnvironment* outer);
struct Object* environmentGet(struct ObjectEnvironment* env, char* key);
struct Object* environmentSet(struct ObjectEnvironment* env, char* key, struct Object* data);
void deleteEnvironment(struct ObjectEnvironment* env);
void deleteAllEnvironment(struct ObjectEnvironment* env);
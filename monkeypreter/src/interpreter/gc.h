#pragma once
#include "object.h"

struct MonkeyGC {
	struct Object* head;
	size_t size;
	size_t maxSize;
};

struct MonkeyGC* createMonkeyGC(void);
void deleteMonkeyGC(struct MonkeyGC* gc);
void addToMonkeyGC(struct MonkeyGC* gc, struct Object* obj);
void markMonkeyObject(struct Object* obj);
void markMonkeyObjectEnvironment(struct ObjectEnvironment* env);
size_t sweepMonkeyGc(struct MonkeyGC* gc);
size_t collectMonkeyGarbage(struct MonkeyGC* gc, struct ObjectEnvironment* env);

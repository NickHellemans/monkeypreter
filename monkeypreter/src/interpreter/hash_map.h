#pragma once
#include "object.h"

#define MAX_KEY_LEN 64

struct HashNode {
	char key[MAX_KEY_LEN];
	struct Object data;
	struct HashNode* next;
};

struct HashMap {
	uint32_t size;
	uint32_t cap;
	struct HashNode** elems;
};

struct HashMap* createHashMap(uint32_t cap);
void destroyHashMap(struct HashMap* hm);
bool insertIntoHashMap(struct HashMap* hm, const char* key, struct Object data);
bool hashMapContains(struct HashMap* hm, const char* key);
struct Object lookupKeyInHashMap(struct HashMap* hm, const char* key);
bool deleteKeyFromHashMap(struct HashMap* hm, const char* key);
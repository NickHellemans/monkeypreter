#pragma once
#include "object.h"

struct HashNode {
	char key[MAX_IDENT_LENGTH];
	void* data;
	struct HashNode* next;
};

struct HashMap {
	uint32_t size;
	uint32_t cap;
	struct HashNode** elems;
};

struct HashMap* createHashMap(uint32_t cap);
void destroyHashMap(struct HashMap* hm);
bool insertIntoHashMap(struct HashMap* hm, const char* key, void* data);
bool hashMapContains(struct HashMap* hm, const char* key);
void* lookupKeyInHashMap(struct HashMap* hm, const char* key);
bool deleteKeyFromHashMap(struct HashMap* hm, const char* key);
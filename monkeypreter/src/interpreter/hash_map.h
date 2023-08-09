#pragma once
#include "object.h"

#define MAX_KEY_LEN 64

enum HashMapKeyType {
	HM_KEY_STRING,
	HM_KEY_EXPRESSION,
};

enum HashMapDataType {
	HM_DATA_OBJECT,
	HM_DATA_EXPRESSION,
};

struct HashNode {
	char key[MAX_KEY_LEN];
	//struct Object* data;
	void* data;
	struct HashNode* next;
};

struct HashMap {
	uint32_t size;
	uint32_t cap;
	struct HashNode** elems;
	//enum HashMapKeyType keyType;
	//enum HashMapDataType dataType;
};

struct HashMap* createHashMap(uint32_t cap);
void destroyHashMap(struct HashMap* hm);
bool insertIntoHashMap(struct HashMap* hm, const char* key, void* data);
bool hashMapContains(struct HashMap* hm, const char* key);
void* lookupKeyInHashMap(struct HashMap* hm, const char* key);
bool deleteKeyFromHashMap(struct HashMap* hm, const char* key);
#include "hash_map.h"
#include <string.h>

static uint32_t getIndex(struct HashMap* hm, const char* key);
static double loadFactor(struct HashMap* hm);
static void rehash(struct HashMap* hm);
static void clear(struct HashMap* hm);

//String hashString
uint32_t hashString(const char* key)
{
	uint32_t hash = 0;
	for (int i = 0; key[i] != '\0'; i++)
	{
		hash = 31 * hash + key[i];
	}
	return hash;
}


struct HashMap* createHashMap(uint32_t cap) {

	struct HashMap* hm = (struct HashMap*)malloc(sizeof * hm);

	if(!hm) {
		perror("malloc (create hashmap) returned `NULL`\n");
		exit(EXIT_FAILURE);
	}

	hm->size = 0;
	hm->cap = cap;
	//hm->keyType = keyType;
	//hm->dataType = dataType;
	hm->elems = (struct HashNode**)calloc(cap, sizeof(struct HashNode*));

	if (!hm) {
		perror("calloc (create hashmap elems) returned `NULL`\n");
		free(hm);
		exit(EXIT_FAILURE);
	}
	return hm;
}

void destroyHashMap(struct HashMap* hm) {
	clear(hm);
	free(hm->elems);
	free(hm);
}

bool insertIntoHashMap(struct HashMap* hm, const char* key, void* data) {

	//if (key == NULL || hm == NULL || hashMapContains(hm, key)) return false;
	if (key == NULL || hm == NULL) return false;

	const uint32_t index = getIndex(hm, key);

	//Rebind value
	if(hashMapContains(hm, key)) {
		struct HashNode* curr = hm->elems[index];

		while (curr != NULL) {
			if (strcmp(curr->key, key) == 0) {
				//free object or not? gc can handle it
				curr->data = data;
				return true;
			}
			curr = curr->next;
		}
	}

	//Create new node
	struct HashNode* node = (struct HashNode*)malloc(sizeof * node);
	if (!node) {
		perror("malloc (insert hashmap node) returned `NULL`\n");
		exit(EXIT_FAILURE);
	}

	strcpy_s(node->key, MAX_IDENT_LENGTH, key);
	node->data = data;

	//Insert at head of bucket with hashed index
	node->next = hm->elems[index];
	hm->elems[index] = node;
	hm->size++;

	if(loadFactor(hm) >= 0.75) {
		rehash(hm);
	}

	return true;
}

bool hashMapContains(struct HashMap* hm, const char* key) {
	if (key == NULL || hm == NULL) return false;

	uint32_t index = getIndex(hm, key);
	struct HashNode* curr = hm->elems[index];

	while(curr != NULL) {
		if(strcmp(curr->key, key) == 0) {
			return true;
		}
		curr = curr->next;
	}

	return false;
}
void* lookupKeyInHashMap(struct HashMap* hm, const char* key) {

	if (key == NULL || hm == NULL || !hashMapContains(hm, key)) return NULL;

	uint32_t index = getIndex(hm, key);
	struct HashNode* curr = hm->elems[index];

	while (curr != NULL) {
		if (strcmp(curr->key, key) == 0) {
			return curr->data;
		}
		curr = curr->next;
	}

	return NULL;
}

bool deleteKeyFromHashMap(struct HashMap* hm, const char* key) {

	if (key == NULL || hm == NULL || !hashMapContains(hm, key)) return false;

	uint32_t index = getIndex(hm, key);

	struct HashNode* curr = hm->elems[index];
	struct HashNode* prev = NULL;

	while (curr != NULL && strcmp(curr->key, key) != 0) {
		prev = curr;
		curr = curr->next;
	}

	if (!curr)
		return false;

	//Deleting head
	if(prev == NULL) {
		hm->elems[index] = curr->next;
	}
	else {
		//Deleting somewhere not at head
		prev->next = curr->next;
	}
	hm->size--;
	free(curr);
	return true;
}

static uint32_t getIndex(struct HashMap* hm, const char* key) {
	return hashString(key) % hm->cap;
}

static double loadFactor(struct HashMap* hm) {
	return (double)hm->size / hm->cap;
}

static void rehash(struct HashMap* hm)
{
	uint32_t oldCapacity = hm->cap;
	struct HashNode** oldTable = hm->elems;
	hm->cap *= 2;
	hm->elems = (struct HashNode**)calloc(hm->cap, sizeof(struct HashNode*));

	if (!hm->elems) {
		perror("calloc (rehash hashmap) returned `NULL` - Abort rehash...\n");
		return;
	}

	for (uint32_t i = 0; i < oldCapacity; i++)
	{
		struct HashNode* curr = oldTable[i];
		while (curr != NULL)
		{
			//Save tmp var to next node before it gets lost
			struct HashNode* next = curr->next;
			uint32_t hashIndex = getIndex(hm, curr->key);
			curr->next = hm->elems[hashIndex];
			hm->elems[hashIndex] = curr;
			curr = next;
		}
	}
	//Only need to delete the array of pointers, not the actual nodes
	free(oldTable);
}

static void clear(struct HashMap* hm)
{
	for (uint32_t i = 0; i < hm->cap; i++)
	{
		struct HashNode* curr = hm->elems[i];
		while (curr != NULL)
		{
			struct HashNode* trash = curr;
			curr = curr->next;
			free(trash);
		}

		hm->elems[i] = NULL;
	}
}
#include "gc.h"
#include <stdio.h>
#include "hash_map.h"

struct MonkeyGC* createMonkeyGC(void) {
	struct MonkeyGC* gc = (struct MonkeyGC*) malloc(sizeof * gc);
	gc->head = NULL;
	gc->size = 0;
	//Trigger GC after 10 objects
	gc->maxSize = 10;
	return gc;
}

void deleteMonkeyGC(struct MonkeyGC* gc) {
	free(gc);
}

void addToMonkeyGC(struct MonkeyGC* monkeyGC, struct Object* obj) {

	//Add to head
	obj->next = monkeyGC->head;
	monkeyGC->head = obj;
	monkeyGC->size++;

	printf("ADDED OBJECT TO GC: \n");
	printf("\t - Type: %s\n", objectTypeToStr(obj->type));
	//printf("\t - Value: %s\n", inspectObject(obj));
	printf("Current GC size = %llu\n", monkeyGC->size);
}

void markMonkeyObject(struct Object* obj) {

	//Handle cycles
	if (obj->mark)
		return;

	obj->mark = true;

	//Mark return object
	if(obj->type == OBJ_RETURN) {
		markMonkeyObject(obj->value.retObj);
	}

	//Mark objects in array
	if(obj->type == OBJ_ARRAY) {
		printf("Marking array objects \n");
		printf("Array mark: %d\n", obj->mark);
		for (size_t i = 0; i < obj->value.arr.size; i ++) {
			printf("Marking object (%p) %llu: %s\n", (void*) obj->value.arr.objects[i], i, inspectObject(obj->value.arr.objects[i]));
			markMonkeyObject(obj->value.arr.objects[i]);
			printf("Object %llu mark = %d\n", i, obj->value.arr.objects[i]->mark);
		}
	}
}

void markMonkeyObjectEnvironment(struct ObjectEnvironment* env) {

	for (uint32_t i = 0; i < env->store->cap; i++)
	{
		struct HashNode* curr = env->store->elems[i];
		while (curr != NULL)
		{
			markMonkeyObject((struct Object*) curr->data);
			curr = curr->next;
		}
	}

	if(env->outer != NULL) {
		markMonkeyObjectEnvironment(env->outer);
	}
}

size_t sweepMonkeyGc(struct MonkeyGC* gc) {
	struct Object** object = &gc->head;
	size_t garbageCounter = 0;
	while (*object != NULL) {
		//Not reached, free and remove from list
		if (!(*object)->mark) {
			struct Object* trash = *object;
			*object = trash->next;
			printf("Collect garbage: \n");
			printf("\t - Type: %s\n", objectTypeToStr(trash->type));
			printf("\t - Value: %s\n", inspectObject(trash));
			printf("Current GC size = %llu\n", gc->size);
			freeObject(trash);
			gc->size--;
			garbageCounter++;
			
		}
		//Reached: unmark for next mark phase
		else {
			//(*object)->mark = false;
			object = &(*object)->next;
		}
	}

	//unmark for next mark phase
	while (*object != NULL) {
		(*object)->mark = false;
		object = &(*object)->next;
	}
	
	return garbageCounter;
}

size_t collectMonkeyGarbage(struct MonkeyGC* gc, struct ObjectEnvironment* env) {
	markMonkeyObjectEnvironment(env);
	printf("MONKEY SWEEP: \n");
	const size_t garbageCount = sweepMonkeyGc(gc);

	printf("Collecting DONE: \n");
	printf("\t - Collected %llu garbage objects \n", garbageCount);
	printf("\t - Current GC size = %llu\n", gc->size);
	return garbageCount;
}
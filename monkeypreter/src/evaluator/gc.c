#include "gc.h"
#include <stdio.h>
#include "hash_map.h"

//Enable - Disable GC logging
//#define LOG_GC 

void markMonkeyObjectEnvironment(struct ObjectEnvironment* env);
size_t sweepMonkeyGc(struct MonkeyGC* gc);

struct MonkeyGC* createMonkeyGC(void) {
	struct MonkeyGC* gc = (struct MonkeyGC*) malloc(sizeof * gc);

	if(!gc) {
		perror("malloc (create gc) returned `NULL`");
		exit(EXIT_FAILURE);
	}

	gc->head = NULL;
	gc->size = 0;
	//Trigger GC after 100 objects
	gc->maxSize = 100;
	return gc;
}

void deleteMonkeyGC(struct MonkeyGC* gc) {

	int counter = 0;
	if (gc->size <= 0) {
		free(gc);
		return;
	}

	struct Object* curr = gc->head;
	while(curr != NULL) {
		struct Object* trash = curr;
		curr = curr->next;
		freeObject(trash);
		counter++;
	}

	printf("Deleted %d objects that were still doing some monkey business\n", counter);
	free(gc);
}

void addToMonkeyGC(struct MonkeyGC* monkeyGC, struct Object* obj) {

	//Add to head
	obj->next = monkeyGC->head;
	monkeyGC->head = obj;
	monkeyGC->size++;

#ifdef LOG_GC
	printf("ADDED OBJECT TO GC: \n");
	printf("\t - Type: %s\n", objectTypeToStr(obj->type));
	printf("Current GC size = %llu\n", monkeyGC->size);
#endif

}

void markMonkeyObject(struct Object* obj) {

	//Handle cycles
	if (obj->mark)
		return;

	obj->mark = true;

#ifdef LOG_GC
	printf("Marking object of type: %s\n", objectTypeToStr(obj->type));
#endif
	//Mark return object
	if(obj->type == OBJ_RETURN) {
		markMonkeyObject(obj->value.retObj);
	}

	//Mark objects in array
	if(obj->type == OBJ_ARRAY) {

		for (size_t i = 0; i < obj->value.arr.size; i ++) {
			markMonkeyObject(obj->value.arr.objects[i]);
		}
	}

	//Mark objects needed in functions (parameters)
	if(obj->type == OBJ_FUNCTION) {
		markMonkeyObjectEnvironment(obj->value.function.env);
		if(obj->value.function.env->outer) {
			markMonkeyObjectEnvironment(obj->value.function.env->outer);
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

#ifdef LOG_GC
			printf("Collect garbage: \n");
			printf("\t - Type: %s\n", objectTypeToStr(trash->type));
			printf("\t - Value: %s\n", inspectObject(trash));
			printf("Current GC size = %llu\n", gc->size);
#endif
			freeObject(trash);
			gc->size--;
			garbageCounter++;
		}
		//Reached: unmark for next mark phase
		else {
#ifdef LOG_GC
			printf("Reached objects: %s\n", objectTypeToStr((* object)->type));
#endif
			//(*object)->mark = false;
			object = &(*object)->next;
		}
	}

	object = &gc->head;
	//unmark for next mark phase
	while (*object != NULL) {
		(*object)->mark = false;
		object = &(*object)->next;
	}
	
	return garbageCounter;
}

size_t collectMonkeyGarbage(struct MonkeyGC* gc, struct ObjectEnvironment* env) {
	markMonkeyObjectEnvironment(env);
#ifdef LOG_GC
	printf("MONKEY SWEEP: \n");
#endif
	const size_t garbageCount = sweepMonkeyGc(gc);
#ifdef LOG_GC
	printf("Collecting DONE: \n");
	printf("\t - Collected %llu garbage objects \n", garbageCount);
	printf("\t - Current GC size = %llu\n", gc->size);
#endif
	return garbageCount;
}
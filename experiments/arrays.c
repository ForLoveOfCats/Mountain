// This file intends to experiment with how the Mountain array type should function internally
// It can be compiled with:
// gcc -g -fsanitize=address -Wall -Wextra -Wpedantic arrays.c -o arrays



#include <stdlib.h>
#include <stdio.h>
#include <assert.h>



struct ArrayIntT
{
	int *head;
	int length;
};


struct ArrayIntT StackArrayInt(int *head, int length) //The space on the stack is at callsite and is provided by the language semantics
{
	struct ArrayIntT instance = {
		head,
		length
	};

	return instance;
}

//This macro emulates the Mountain stack array semantics
#define STACK_ARRAY_INT(NAME, LENGTH) \
	int NAME##_ARRAY_INT_STACK_STORE[LENGTH]; \
	struct ArrayIntT NAME = StackArrayInt(NAME##_ARRAY_INT_STACK_STORE, LENGTH); \


struct ArrayIntT *HeapArrayInt(int length) //The array itself and ArrayIntT are both on the heap
{
	struct ArrayIntT *instance = malloc(sizeof(struct ArrayIntT));
	instance->head = malloc(sizeof(int) * length);
	instance->length = length;

	return instance;
}

void HeapArrayIntDeconstructor(struct ArrayIntT *instance)
{
	free(instance->head);
	free(instance);
}


int main()
{
	STACK_ARRAY_INT(stack_array_var, 5); /* In Mountain this would be as follows?
	let:i32:Array stack_array_var = i32:Array::(5);
		or
	let stack_array_var = i32:Array::(5); */
	printf("Stack allocated array of ints\n");
	for(int index = 0; index <= 4; index++)
	{
		assert(index <= stack_array_var.length-1);
		stack_array_var.head[index] = index; //The head[index] will be hidden by the .At and .Set of the Mountain array
		printf("%i\n", stack_array_var.head[index]);
	}


	printf("\n");


	printf("Heap allocated array of ints\n");
	struct ArrayIntT *heap_array_var = HeapArrayInt(5);/* In Mountain this would be as follows
	let:i32:Array:Ptr heap_array_var = i32:Array::heap(5);
		or
	let heap_array_var = i32:Array::heap(5); */
	for(int index = 0; index <= 4; index++)
	{
		assert(index <= heap_array_var->length-1);
		heap_array_var->head[index] = index;
		printf("%i\n", stack_array_var.head[index]);
	}
	HeapArrayIntDeconstructor(heap_array_var);
}

//This file intends to experiment with how the Mountain T:Array type will function internally
//The T:Array type assumes that the size will always be known at compile time
//When the size needs to be caculated at compile time one would instead use T:HeapArray
//This T:HeapArray type amounts to little more than a size and a pointer to a malloced array

//Compile with:
// gcc -g -fsanitize=address -fsanitize=undefined -Wall -Wextra -Wpedantic arrays.c -o arrays
//or
// clang -g -fsanitize=address -fsanitize=undefined -Wall -Wextra -Wpedantic arrays.c -o arrays



//The below code is roughly equivalent to the following Mountain code
//(If you spot any differences between it and the below C code please open a bug)
#if 0 //Change to a 1 to get syntax highlighting in your editor
module Main;


func:Void main() {
	{
		let array = new:i32:Array(5)::();

		for(let index = 0; x <= 4, x += 1) {
			array.at(x) |= x; // T:Array.at(i32) returns a T:Ptr
		}

		println(array.at(x)|to_string());
	}

	{
		let array = [0, 1, 2, 3, 4];
		println(array.at(x)|to_string());
	}

	{
		let array = new:i32:Array(5)::heap();

		for(let index = 0; x <= 4, x += 1) {
			array|at(x) |= x;
		}

		println(array|at(x)|to_string());
		array.free();
	}

	{
		let array = new:i32:Array(5)::heap([0, 1, 2, 3, 4]);
		//Sidenote: This constructor would not have the cost of passing the stack array as all constructors
		// are macros in Mountain so this would deconstruct to something very efficient
		println(array|at(x)|to_string());
		array.free();
	}
}
#endif



#include <stdlib.h>
#include <stdio.h>
#include <assert.h>



//The Mountain compiler would generate a version of this for every type and length combination used
struct IntArray5
{
	int head[5];
};


//The use of macros here is simply to "simulate" the Mountain Array semantics
//Some of them could use __VA_ARGS__ but do not in order to clarify that this would all be handled by code generation

#define STACK_INT_ARRAY_5_EMPTY(name) struct IntArray5 name
#define STACK_INT_ARRAY_5_DEFAULT_VALUES(name, item0, item1, item2, item3, item4) struct IntArray5 name = { {item0, item1, item2, item3, item4} }

#define HEAP_INT_ARRAY_5_EMPTY(name) struct IntArray5 *name = (struct IntArray5*)malloc(sizeof(struct IntArray5))
#define HEAP_INT_ARRAY_5_DEFAULT_VALUES(name, item0, item1, item2, item3, item4) struct IntArray5 *name = (struct IntArray5*)malloc(sizeof(struct IntArray5)); \
	{ \
		name->head[0] = item0; \
		name->head[1] = item1; \
		name->head[2] = item2; \
		name->head[3] = item3; \
		name->head[4] = item4; \
	} \


#define STACK_INT_ARRAY_AT(array, index) array.head[index]
#define HEAP_INT_ARRAY_AT(array, index) array->head[index]


int main()
{
	{
		//Stack allocated array of 5 ints filled with garbage data
		STACK_INT_ARRAY_5_EMPTY(array); //let array = new:i32:Array(5)::();
		for(int x = 0; x <= 4; x++)
			STACK_INT_ARRAY_AT(array, x) = x; //array.at(x) |= x;
		for(int x = 0; x <= 4; x++) //println(array.at(x)|to_string());
			printf("%i\n", STACK_INT_ARRAY_AT(array, x));
	}


	printf("\n");


	{
		//Stack allocated array of 5 ints filled with default data
		STACK_INT_ARRAY_5_DEFAULT_VALUES(array, 0, 1, 2, 3, 4); //let array = [0, 1, 2, 3, 4];
		for(int x = 0; x <= 4; x++) //println(array.at(x)|to_string());
			printf("%i\n", STACK_INT_ARRAY_AT(array, x));
	}


	printf("\n");


	{
		//Heap allocated array of 5 ints filled with garbage data
		HEAP_INT_ARRAY_5_EMPTY(array); //let array = new:i32:Array(5)::heap();
		for(int x = 0; x <= 4; x++)
			HEAP_INT_ARRAY_AT(array, x) = x; //array|at(x) |= x;
		for(int x = 0; x <= 4; x++) //println(array|at(x)|to_string());
			printf("%i\n", HEAP_INT_ARRAY_AT(array, x));
		free(array); //array.free();
	}


	printf("\n");


	{
		//Heap allocated array of 5 ints filled with default data
		HEAP_INT_ARRAY_5_DEFAULT_VALUES(array, 0, 1, 2, 3, 4); //let array = new:i32:Array(5)::heap([0, 1, 2, 3, 4]);
		for(int x = 0; x <= 4; x++) //println(array|at(x)|to_string());
			printf("%i\n", HEAP_INT_ARRAY_AT(array, x));
		free(array); //array.free();
	}
}

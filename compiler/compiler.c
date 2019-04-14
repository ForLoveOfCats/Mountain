#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>


int main(int arg_count, char *arg_array[])
{
	if(arg_count != 2)
	{
		printf("Please provide a file path to compile\n");
		return EXIT_FAILURE;
	}

	FILE *source_file = fopen(arg_array[1], "r");
	if(source_file == NULL)
	{
		printf("Could not open file '%s' to compile\n", arg_array[1]);
		return EXIT_FAILURE;
	}

	char car;
	while(true)
	{
		car = fgetc(source_file);
		if(car == EOF)
			break;

		printf("%c", car);
	}

	fclose(source_file);
	return EXIT_SUCCESS;
}


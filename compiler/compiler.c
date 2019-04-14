#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>


char *next_token(FILE *source_file)
{
	bool in_token = false;
	char *token = malloc(sizeof(char));
	char car;
	while(true)
	{
		car = fgetc(source_file);
		if(car == EOF)
		{
			goto return_token;
		}

		if(car == ' ' || car == '\t' || car == '\n')
		{
			if(!in_token)
				continue;
			else
				goto return_token;
		}
		else
			in_token = true;

		int old_token_length = strlen(token);
		char *new_token = malloc(sizeof(char) * (old_token_length + 1));
		strncpy(new_token, token, old_token_length);
		free(token);
		token = new_token;
		token[old_token_length] = car;
		token[old_token_length + 1] = '\0';
	}

return_token:
	if(strlen(token) <= 1 && car == EOF) //If token is blank and is at the end of the file
	{
		return NULL; //Return NULL
	}
	return token; //Otherwise return the token
}


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

	while(true)
	{
		char *token = next_token(source_file);
		if(token == NULL)
			break;

		printf("%s\n", token);
	}

	fclose(source_file);
	return EXIT_SUCCESS;
}


#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>


enum TOKEN_TYPE {TOKEN_WORD};

struct TOKEN
{
	bool valid;
	enum TOKEN_TYPE type;
	char *string;
};

struct TOKEN next_token(FILE *source_file)
{
	struct TOKEN token = {true, TOKEN_WORD, strdup("")};
	bool in_token = false;
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

		int old_token_length = strlen(token.string);
		char *new_token = malloc(sizeof(char) * (old_token_length+2)); //Two larger than originally
		strcpy(new_token, token.string);
		free(token.string);
		token.string = new_token;
		token.string[old_token_length] = car; //Place in first new spot
		token.string[old_token_length + 1] = '\0'; //Null terminate in second new spot
	}

return_token: ;
	if(strlen(token.string) <= 0 && car == EOF) //If token is blank and is at the end of the file
	{
		token.valid = false;
		return token; //Return NULL
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

	struct TOKEN token = {false, 0, strdup("")};
	while(true)
	{
		free(token.string);
		token = next_token(source_file);
		if(!token.valid)
			break;

		if(token.type == TOKEN_WORD)
			printf("Word found: %s\n", token.string);
	}
	free(token.string);

	fclose(source_file);
	return EXIT_SUCCESS;
}


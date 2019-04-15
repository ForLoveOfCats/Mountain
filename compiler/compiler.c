#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>


struct TOKEN
{
	bool valid;
	char *string;
};


struct TOKEN next_token(FILE *source_file)
{
	bool in_token = false;
	char *token = malloc(sizeof(char));
	token[0] = '\0';
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
		char *new_token = malloc(sizeof(char) * (old_token_length+2)); //Two larger than originally
		strcpy(new_token, token);
		free(token);
		token = new_token;
		token[old_token_length] = car; //Place in first new spot
		token[old_token_length + 1] = '\0'; //Null terminate in second new spot
	}

return_token: ;
	struct TOKEN token_struct = {true, token};
	if(strlen(token) <= 1 && car == EOF) //If token is blank and is at the end of the file
	{
		token_struct.valid = false;
		return token_struct; //Return NULL
	}
	return token_struct; //Otherwise return the token
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

	struct TOKEN token = {false, malloc(sizeof(char))};
	while(true)
	{
		free(token.string);
		token = next_token(source_file);
		if(!token.valid)
			break;

		printf("%s\n", token.string);
	}
	free(token.string);

	fclose(source_file);
	return EXIT_SUCCESS;
}


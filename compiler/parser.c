#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "parser.h"
#include "ast.h"


char *token_type_name[] = {"WORD", "COLON", "SEMICOLON", "COMMENT", "EQUALS"};


void free_token(struct TOKEN token) //In case I add more fields which require freeing
{
	free(token.string);
}


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

		if(!in_token)
		{
			switch(car)
			{
				case ':':
				{
					token.type = TOKEN_COLON;
					goto return_car_as_token;
				}

				case ';':
				{
					token.type = TOKEN_SEMICOLON;
					goto return_car_as_token;
				}

				case '#':
				{
					token.type = TOKEN_COMMENT;
					goto return_car_as_token;
				}

				case '=':
				{
					token.type = TOKEN_EQUALS;
					goto return_car_as_token;
				}
			}
		}

		switch(car)
		{
			case ' ':
			case '\t':
			case '\n':
			case ':':
			case ';':
			case '#':
			case '=':
			{
				if(!in_token)
					continue;
				else
				{
					ungetc(car, source_file); //HACK FIXME
					goto return_token;
				}

				break;
			}
			default:
				in_token = true;
		}

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

return_car_as_token: ;
	free_token(token);
	token.string = malloc(sizeof(char) * 2);
	token.string[0] = car;
	token.string[1] = '\0';
	return token;
}


struct TOKEN next_token_expect(FILE* source_file, enum TOKEN_TYPE expected)
{
	struct TOKEN token = next_token(source_file);

	if(token.type != expected)
	{
		printf("Expected %s: found %s\n", token_type_name[expected], token_type_name[token.type]);
		exit(EXIT_FAILURE);
	}

	return token;
}


struct NODE *parse_next_expression(FILE *source_file)
{
	//NOTE TODO FIXME HACK
	//For now we are just assuming that any expression is an integer
	//this is VERY BAD(tm) and should be improved ASAP

	struct NODE *node = create_node(AST_INT);
	struct TOKEN token = next_token_expect(source_file, TOKEN_WORD);
	node->int_value = (int32_t)strtol(token.string, NULL, 10);
	free_token(token);
	return node;
}


bool parse_next_statement(FILE *source_file)
{
	struct TOKEN token = next_token(source_file);
	if(!token.valid)
	{
		printf("Reached end of file\n");
		free_token(token);
		return false;
	}

	if(token.type == TOKEN_COMMENT)
	{
		printf("Encountered comment, reading to end of line\n");
		free_token(token);

		//Consume until end of comment
		char car = ' ';
		while(car != '\n')
		{
			car = getc(source_file);
		}

		return true;
	}

	if(token.type == TOKEN_SEMICOLON)
	{
		printf("Reached end of statement\n");
		free_token(token);
		return true;
	}

	if(token.type == TOKEN_WORD)
	{
		printf("Word found: %s\n", token.string);
		if(strcmp(token.string, "def") == 0)
		{
			free_token(next_token_expect(source_file, TOKEN_COLON));

			//We must be dealing with a variable definition
			struct NODE *new_node = create_node(AST_DEF);
			new_node->def_location = current_parse_parent_node->stack_len;
			current_parse_parent_node->stack_len += 1;

			free(new_node->type_name);
			new_node->type_name = next_token_expect(source_file, TOKEN_WORD).string;

			free(new_node->def_name);
			new_node->def_name = next_token_expect(source_file, TOKEN_WORD).string;

			free_token(next_token_expect(source_file, TOKEN_EQUALS));

			add_node(new_node, parse_next_expression(source_file));

			add_node(current_parse_parent_node, new_node);
		}
	}
	else if(token.type == TOKEN_COLON)
		printf("Colon found\n");

	free_token(token);
	return true;
}

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "compiler.h"
#include "parser.h"
#include "ast.h"


#define NEXT_TOKEN(token) \
	token = token->next_token; \
	if(token == NULL) \
	{ \
		printf("Parse error: Encountered end of file unexpectedly\n"); \
		exit(EXIT_FAILURE); \
	} \


char *token_type_name[] = { FOREACH_TOKEN_TYPE(GENERATE_STRING) };


int current_token_start = 0;


void free_token(struct TOKEN *token) //Frees a token and all next tokens
{
	free(token->string);
	if(token->next_token != NULL)
		free_token(token->next_token);
	free(token);
}


struct TOKEN *next_token_from_file(FILE *source_file)
{
	struct TOKEN *token = malloc(sizeof(struct TOKEN));
	token->valid = true;
	token->type = TOKEN_WORD;
	token->string = strdup("");
	token->next_token = NULL;

	bool in_token = false;
	char car;
	while(true)
	{
		car = fgetc(source_file);
		current_file_character++;
		if(car == EOF)
		{
			goto return_token;
		}
		else if(car == '\n')
		{
			current_file_line++;
			current_file_character = 0;
		}

		if(!in_token)
		{
			switch(car)
			{
				case ':':
				{
					token->type = TOKEN_COLON;
					goto return_car_as_token;
				}

				case ';':
				{
					token->type = TOKEN_SEMICOLON;
					goto return_car_as_token;
				}

				case '#':
				{
					printf("Encountered comment, reading to end of line\n");
					while(car != '\n')
					{
						car = getc(source_file);
					}
					current_file_line++;
					current_file_character = 0;
					break;
				}

				case '=':
				{
					token->type = TOKEN_EQUALS;
					goto return_car_as_token;
				}

				case '(':
				{
					token->type = TOKEN_OPEN_PARENTHESES;
					goto return_car_as_token;
				}

				case ')':
				{
					token->type = TOKEN_CLOSE_PARENTHESES;
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
			case '(':
			case ')':
			{
				if(!in_token)
					continue;
				else
				{
					ungetc(car, source_file); //HACK FIXME
					current_file_character--;
					if(car == '\n')
						current_file_line--;

					goto return_token;
				}

				break;
			}
			default:
				if(!in_token)
					current_token_start = current_file_character;
				in_token = true;
		}

		int old_token_length = strlen(token->string);
		char *new_token = malloc(sizeof(char) * (old_token_length+2)); //Two larger than originally
		strcpy(new_token, token->string);
		free(token->string);
		token->string = new_token;
		token->string[old_token_length] = car; //Place in first new spot
		token->string[old_token_length + 1] = '\0'; //Null terminate in second new spot
	}

return_token: ;
	if(strlen(token->string) <= 0 && car == EOF) //If token is blank and is at the end of the file
	{
		token->valid = false;
		return token; //Return NULL
	}
	return token; //Otherwise return the token

return_car_as_token: ;
	current_token_start = current_file_character;
	free(token->string);
	token->string = malloc(sizeof(char) * 2);
	token->string[0] = car;
	token->string[1] = '\0';
	return token;
}


struct TOKEN *tokenize_file(FILE *source_file)
{
	int token_count = 1;
	struct TOKEN *first_token = next_token_from_file(source_file);
	struct TOKEN *token = first_token;
	while(token->valid)
	{
		struct TOKEN *next_token = next_token_from_file(source_file);
		token->next_token = next_token;
		token = next_token;
		token_count++;
	}

	return first_token;
}


void expect(struct TOKEN *token, enum TOKEN_TYPE expected)
{
	if(token->type != expected)
	{
		printf("Parse error on line %i, column %i. Expected %s: found %s\n", current_file_line, current_token_start, token_type_name[expected], token_type_name[token->type]);
		exit(EXIT_FAILURE);
	}
}


int is_number(const char *s) //Adapted from rosettacode.org
{
	if(s == NULL || *s == '\0' || *s == ' ')
		return 0;

	char *p;
	strtod(s, &p);
	return *p == '\0';
}


struct NODE *parse_next_expression(struct TOKEN *token)
{
	//NOTE TODO FIXME HACK
	//For now we are just assuming that any expression is an integer
	//this is VERY BAD(tm) and should be improved ASAP

	struct NODE *expression_root = create_node(AST_ROOT); //NOTE AST type is set later

	while(token->type != TOKEN_SEMICOLON)
	{
		if(is_number(token->string))
		{
			expression_root->type = AST_LITERAL;
			expression_root->literal_type = LITERAL_INT;
			free(expression_root->literal_string);
			expression_root->literal_string = strdup(token->string);
		}
		else
		{
			printf("Found non-numerical literal parsing expression on line %i, column %i: only integer literals are currently supported, sorry\n",
			       current_file_line, current_token_start);
			exit(EXIT_FAILURE);
		}

		token = token->next_token;
	}

	if(expression_root->type == AST_ROOT) //Defensive programming, fail early
	{
		printf("INTERNAL ERROR: expression_root is still of type AST_ROOT\n");
		exit(EXIT_FAILURE);
	}
	return expression_root;
}


bool parse_next_statement(struct TOKEN *token)
{
	if(!token->valid)
	{
		printf("Reached end of file\n");
		return false;
	}

	if(token->type == TOKEN_SEMICOLON)
	{
		printf("Reached end of statement\n");
		return true;
	}

	if(token->type == TOKEN_WORD)
	{
		printf("Word found: %s\n", token->string);
		if(strcmp(token->string, "def") == 0)
		{
			NEXT_TOKEN(token);
			expect(token, TOKEN_COLON);
			NEXT_TOKEN(token);

			//We must be dealing with a variable definition
			struct NODE *new_node = create_node(AST_DEF);
			new_node->def_location = current_parse_parent_node->stack_len;
			current_parse_parent_node->stack_len += 1;

			free(new_node->type_name);
			expect(token, TOKEN_WORD);
			new_node->type_name = strdup(token->string);
			NEXT_TOKEN(token);

			free(new_node->def_name);
			expect(token, TOKEN_WORD);
			new_node->def_name = strdup(token->string);
			NEXT_TOKEN(token);

			expect(token, TOKEN_EQUALS);
			NEXT_TOKEN(token);

			add_node(new_node, parse_next_expression(token)); //Should read to semicolon

			add_node(current_parse_parent_node, new_node);
		}
	}
	else if(token->type == TOKEN_COLON)
		printf("Colon found\n");

	return true;
}

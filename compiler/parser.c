#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "compiler.h"
#include "parser.h"
#include "ast.h"


#define NEXT_TOKEN(token) \
	if(token->next == NULL) \
	{ \
		printf("Parse error: Encountered end of file unexpectedly on line %i, column %i\n", token->line_number, token->end_char + 1); \
		exit(EXIT_FAILURE); \
	} \
	token = token->next; \


char *token_type_name[] = { FOREACH_TOKEN_TYPE(GENERATE_STRING) };


int current_token_start = 0;


void free_token_list(struct TOKEN *token) //Frees a token and all next tokens
{
	free(token->string);
	if(token->next != NULL)
		free_token_list(token->next);
	free(token);
}


struct TOKEN *next_token_from_file(FILE *source_file)
{
	struct TOKEN *token = malloc(sizeof(struct TOKEN));
	token->type = TOKEN_WORD;
	token->string = strdup("");
	token->next = NULL;

	bool in_token = false;
	char car;
	while(true)
	{
		car = fgetc(source_file);
		current_file_character++;
		if(car == EOF)
		{
			ungetc(car, source_file);
			current_file_character--;
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

				case '{':
				{
					token->type = TOKEN_OPEN_BRACE;
					goto return_car_as_token;
				}

				case '}':
				{
					token->type = TOKEN_CLOSE_BRACE;
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
			case '{':
			case '}':
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
		free_token_list(token);
		return NULL;
	}
	token->line_number = current_file_line;
	token->start_char = current_token_start;
	token->end_char = current_file_character;
	return token;

return_car_as_token: ;
	current_token_start = current_file_character;
	free(token->string);
	token->string = malloc(sizeof(char) * 2);
	token->string[0] = car;
	token->string[1] = '\0';
	token->line_number = current_file_line;
	token->start_char = current_token_start;
	token->end_char = current_file_character;
	return token;
}


struct TOKEN *tokenize_file(FILE *source_file)
{
	int token_count = 1;
	struct TOKEN *first_token = next_token_from_file(source_file);
	struct TOKEN *token = first_token;
	while(token != NULL)
	{
		struct TOKEN *next_token = next_token_from_file(source_file);
		token->next = next_token;
		token = next_token;
		token_count++;
	}

	return first_token;
}


void expect(struct TOKEN *token, enum TOKEN_TYPE expected)
{
	if(token->type != expected)
	{
		printf("Parse error on line %i, column %i. Expected %s: found %s\n", token->line_number, token->start_char, token_type_name[expected], token_type_name[token->type]);
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


struct NODE *parse_next_expression(struct TOKEN **token)
{
	//NOTE TODO FIXME HACK
	//For now we are just assuming that any expression is an integer
	//this is VERY BAD(tm) and should be improved ASAP

	struct NODE *expression_root = create_node(AST_BLOCK, (*token)->line_number); //NOTE AST type is set later

	while((*token)->type != TOKEN_SEMICOLON)
	{
		if(is_number((*token)->string))
		{
			expression_root->type = AST_LITERAL;
			expression_root->literal_type = LITERAL_INT;
			free(expression_root->literal_string);
			expression_root->literal_string = strdup((*token)->string);
			free(expression_root->type_name);
			expression_root->type_name = strdup("Int32"); //HACK FIXME This is the set type hardcoded to Int32
		}
		else
		{
			printf("Found non-numerical literal parsing expression on line %i, column %i: only integer literals are currently supported, sorry\n",
			       current_file_line, current_token_start);
			exit(EXIT_FAILURE);
		}

		(*token) = (*token)->next;
	}

	assert(expression_root->type != AST_BLOCK);
	return expression_root;
}


struct TOKEN *parse_next_statement(struct TOKEN *token)
{
	if(token == NULL)
	{
		printf("Reached end of file\n");
		return NULL;
	}

	if(token->type == TOKEN_SEMICOLON)
	{
		printf("Reached end of statement\n");
		return token;
	}

	if(token->type == TOKEN_WORD)
	{
		if(strcmp(token->string, "var") == 0) //mutable variable declaration
		{
			struct NODE *new_node = create_node(AST_VAR, token->line_number);

			NEXT_TOKEN(token);
			expect(token, TOKEN_COLON);

			NEXT_TOKEN(token);
			expect(token, TOKEN_WORD);
			free(new_node->type_name);
			new_node->type_name = strdup(token->string);

			NEXT_TOKEN(token);
			free(new_node->variable_name);
			expect(token, TOKEN_WORD);
			new_node->variable_name = strdup(token->string);

			NEXT_TOKEN(token);
			expect(token, TOKEN_EQUALS);

			NEXT_TOKEN(token);
			add_node(new_node, parse_next_expression(&token)); //Should read to semicolon

			add_node(current_parse_parent_node, new_node);

			if(token->next == NULL)
				return NULL;
			NEXT_TOKEN(token);
		}

		else //It must be a variable set or a function call   TODO Add function calls
		{
			//HACK for now we are just assuming that it is a variable set
			expect(token, TOKEN_WORD);
			struct NODE *new_node = create_node(AST_SET, token->line_number);
			free(new_node->variable_name);
			new_node->variable_name = strdup(token->string);

			NEXT_TOKEN(token);
			expect(token, TOKEN_EQUALS);

			NEXT_TOKEN(token);
			struct NODE *expression = parse_next_expression(&token);
			add_node(new_node, expression); //Should read to semicolon
			free(new_node->type_name);
			new_node->type_name = strdup(expression->type_name);

			add_node(current_parse_parent_node, new_node);

			if(token->next == NULL)
				return NULL;
			NEXT_TOKEN(token);
		}
	}

	return token;
}


struct TOKEN *parse_block(struct TOKEN *token, int level)
{
	assert(level >= 0);
	bool inner_block = level > 0;

	if(inner_block)
	{
		struct NODE *block = create_node(AST_BLOCK, token->line_number);
		add_node(current_parse_parent_node, block);
		current_parse_parent_node = block;
	}

	while(true)
	{
		if(token->type == TOKEN_OPEN_BRACE)
		  token = parse_block(token->next, level + 1);
		if(token == NULL)
			break;

		token = parse_next_statement(token);
		if(token == NULL)
			break;

		else if(token->type == TOKEN_CLOSE_BRACE)
		{
			if(!inner_block)
			{
				printf("Parse error @ line %i column %i: Unexpected close brace\n", token->line_number, token->start_char);
				exit(EXIT_FAILURE);
			}

			current_parse_parent_node = current_parse_parent_node->parent;
			return token->next;
		}
	}

	//At this point we know that we have reached the end of the file
	if(inner_block)
	{
		printf("Parse error: Expected close brace, found end of file\n");
		exit(EXIT_FAILURE);
	}
	return NULL;
}

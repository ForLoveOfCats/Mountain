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

				case '+':
				{
					token->type = TOKEN_ADD;
					goto return_car_as_token;
				}

				case '-':
				{
					token->type = TOKEN_SUB;
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
			case '+':
			case '-':
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


bool token_is_op(struct TOKEN *token)
{
	switch(token->type)
	{
		case TOKEN_ADD:
			return true;

		case TOKEN_SUB:
			return true;

 		default:
			return false;
	}
}


int get_op_precedence(enum OP_TYPE op)
{
	switch(op)
	{
		case OP_ADD:
			return 0;
			break;

		case OP_SUB:
			return 0;

		default:
			printf("INTERNAL ERROR: Unimplimented op precedence\n");
			exit(EXIT_FAILURE);
	}
}


void parse_expression_bounds(struct NODE *root, struct TOKEN *start, struct TOKEN *end) //NOTE: these bounds are inclusive
{
	//TODO: We need to eventually handle periods in numbers once periods are their own token

	enum LAST_TYPE {NONE, VALUE, OP};

	struct NODE *current_op_node = NULL;
	struct NODE *left_value_node = NULL;

	struct TOKEN *token = start;
	int value_count = 0;
	bool last_was_end = false; //I hate this
	enum LAST_TYPE last_type = NONE;

	while(true)
	{
		if(last_was_end) //This sucks
		  break; //Oh god why
		printf("Iterating %s\n", token->string);

		if(!token_is_op(token)) //is a value
		{
			//TODO FIXME right now we assume that it is a `Number`
			assert(is_number(token->string));

			if(last_type == VALUE)
			{
				printf("Parse error @ line %i column %i: Expected operator, found value '%s'\n",
				       token->line_number, token->start_char, token->string);
				exit(EXIT_FAILURE);
			}

			struct NODE *new_node = create_node(AST_LITERAL, token->line_number);
			new_node->literal_type = LITERAL_NUMBER;
			free(new_node->literal_string);
			new_node->literal_string = strdup(token->string);

			if(left_value_node == NULL)
				left_value_node = new_node;
			else
			{
				add_node(current_op_node, new_node);
				left_value_node = NULL;
			}

			printf("Was value\n");
			last_type = VALUE;
			value_count++;
		}
		else //is an op
		{
			if(token->type == TOKEN_SUB && value_count == 0) //leading minus for negative number
			{
				printf("Parse warning @ line %i column %i: Negative numbers are not currently supported\n",
				       token->line_number, token->start_char);
				//TODO: Add negative number support
			}
			else
			{
				if(last_type != VALUE)
				{
					printf("Parse error @ line %i column %i: Expected value, found '%s'\n",
					       token->line_number, token->start_char, token->string);
					exit(EXIT_FAILURE);
				}

				struct NODE *new_node = create_node(AST_OP, token->line_number);
				switch(token->type)
				{
					case TOKEN_ADD:
						new_node->op_type = OP_ADD;
						break;

					case TOKEN_SUB:
						new_node->op_type = OP_SUB;
						break;

					default:
						printf("Parse error @ line %i column %i: Cannot convert operation token to operation\n",
							   token->line_number, token->start_char);
						exit(EXIT_FAILURE);
				}

				if(current_op_node == NULL)
				{
					current_op_node = new_node;
					add_node(current_op_node, left_value_node);
				}
				else
				{
					if(get_op_precedence(current_op_node->op_type) >= get_op_precedence(new_node->op_type))
					{
						struct NODE *old_op_node = current_op_node;
						current_op_node = new_node;
						add_node(current_op_node, old_op_node);
						left_value_node = old_op_node;
					}
				}

				last_type = OP;
			}
		}

		last_was_end = token == end; //End my suffering
		token = token->next;
	}

	if(value_count == 1)
	{
		assert(left_value_node != NULL);
		add_node(root, left_value_node);
	}
	else
	{
		assert(current_op_node != NULL);
		printf("%i\n", recursive_count_node_children(current_op_node));
		add_node(root, current_op_node);
	}
}


struct NODE *parse_expression_to_semicolon(struct TOKEN **token) //TODO: Error on no token before semicolon
{
	struct NODE *expression_root = create_node(AST_EXPRESSION, (*token)->line_number);

	//TODO count parenthesis to validate
	struct TOKEN *start = *token;
	struct TOKEN *end = *token;
	while((*token)->type != TOKEN_SEMICOLON)
	{
		end = *token;

		*token = (*token)->next;
		if(token == NULL) //We know we reached the end of the file before hitting a semicolon
		{
			printf("Parse error: Expected semicolon, found end of file\n");
			exit(EXIT_FAILURE);
		}
	}

	parse_expression_bounds(expression_root, start, end);
	return expression_root;
}


struct NODE *parse_next_expression(struct TOKEN **token) //NOTE unused, currently kept for reference
{
	//NOTE TODO FIXME HACK
	//For now we are just assuming that any expression is a number or math expression
	//this is VERY BAD(tm) and should be improved ASAP

	int node_count = 0;
	struct NODE *expression_root = create_node(AST_EXPRESSION, (*token)->line_number);
	struct NODE *current_parent = expression_root;

	while((*token)->type != TOKEN_SEMICOLON)
	{
		if(token_is_op(*token))
 		{
			/*if(node_count % 2 == 0)
			{
				printf("Parse error @ line %i column %i: Expected value, found '%s'\n",
				       (*token)->line_number, (*token)->start_char, (*token)->string);
				exit(EXIT_FAILURE);
			}*/

			struct NODE *new_node = create_node(AST_OP, (*token)->line_number);
			switch((*token)->type)
			{
				case TOKEN_ADD:
					new_node->op_type = OP_ADD;
					break;

				case TOKEN_SUB:
					new_node->op_type = OP_SUB;
					break;

				default:
					printf("INTERNAL ERROR: op_type was not set but should have been\n");
					exit(EXIT_FAILURE);
			}
			add_node(current_parent, new_node);
			current_parent = new_node;
			node_count++;
		}
		else //Number, variable get, or function call
		{
		  assert(is_number((*token)->string));
		  struct NODE *new_node = create_node(AST_LITERAL, (*token)->line_number);
		  free(new_node->type_name);
		  new_node->type_name = strdup("Number");
		  add_node(current_parent, new_node);
		  node_count++;
		}


		(*token) = (*token)->next;
	}

	if(node_count <= 0)
	{
		printf("Parse error @ line %i column %i: Expected value, found semicolon\n",
		       (*token)->line_number, (*token)->start_char);
		exit(EXIT_FAILURE);
	}

	/*while((*token)->type != TOKEN_SEMICOLON)
	{
		if(is_number((*token)->string))
		{
			expression_root->type = AST_LITERAL;
			expression_root->literal_type = LITERAL_INT;
			free(expression_root->literal_string);
			expression_root->literal_string = strdup((*token)->string);
			free(expression_root->type_name);
			expression_root->type_name = strdup("Number"); //HACK FIXME type is hardcoded to untyped Number
		}
		else
		{
			printf("Found non-numerical literal parsing expression on line %i, column %i: only integer literals are currently supported, sorry\n",
			       current_file_line, current_token_start);
			exit(EXIT_FAILURE);
		}

		(*token) = (*token)->next;
	}*/

	assert(expression_root->type != AST_BLOCK);
	free(expression_root->type_name);
	expression_root->type_name = strdup(expression_root->first_child->type_name);

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
			add_node(new_node, parse_expression_to_semicolon(&token));

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
			struct NODE *expression = parse_expression_to_semicolon(&token);
			add_node(new_node, expression);
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

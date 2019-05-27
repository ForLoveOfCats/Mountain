#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "compiler.h"
#include "parser.h"
#include "ast.h"


#define PARSE_ERROR_LC(line, column, ...) \
	{ \
		printf("Parse error @ line %i column %i: ", line, column); \
		printf( __VA_ARGS__); \
		printf("\n"); \
		exit(EXIT_FAILURE); \
	} \


#define PARSE_ERROR(...) \
	{ \
		printf("Parse error: "); \
		printf( __VA_ARGS__); \
		printf("\n"); \
		exit(EXIT_FAILURE); \
	} \


#define NEXT_TOKEN(token) \
	{ \
		if(token->next == NULL) \
		{ \
			printf("Parse error: Encountered end of file unexpectedly on line %i, column %i\n", token->line_number, token->end_char + 1); \
			exit(EXIT_FAILURE); \
		} \
		token = token->next; \
	} \


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
					char next_car = fgetc(source_file);
					if(next_car != '=')
					{
						ungetc(next_car, source_file); //HACK FIXME
						token->type = TOKEN_EQUALS;
						goto return_car_as_token;
					}

					token->type = TOKEN_TEST_EQUAL;
					free(token->string);
					token->string = strdup("==");
					goto return_token;
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

				case '*':
				{
					token->type = TOKEN_MUL;
					goto return_car_as_token;
				}

				case '/':
				{
					token->type = TOKEN_DIV;
					goto return_car_as_token;
				}

				case '!':
				{
					token->type = TOKEN_EXCLAMATION;
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
			case '*':
			case '/':
			case '!':
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
		PARSE_ERROR_LC(token->line_number, token->start_char, "Expected %s but found %s", token_type_name[expected], token_type_name[token->type]);
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
		case TOKEN_TEST_EQUAL:
		case TOKEN_ADD:
		case TOKEN_SUB:
		case TOKEN_MUL:
		case TOKEN_DIV:
			return true;

 		default:
			return false;
	}
}


bool token_is_unop(struct TOKEN *token)
{
	switch(token->type)
	{
		case TOKEN_EXCLAMATION:
			return true;

 		default:
			return false;
	}
}


int get_op_precedence(enum OP_TYPE op)
{
	switch(op)
	{
		case OP_TEST_EQUAL:
			return 1;

		case OP_ADD:
		case OP_SUB:
			return 2;

		case OP_MUL:
		case OP_DIV:
			return 3;

		default:
			printf("INTERNAL ERROR: Unimplimented op precedence\n");
			exit(EXIT_FAILURE);
	}
}


void parse_expression_bounds(struct NODE *root, struct TOKEN *start, struct TOKEN *end) //NOTE: these bounds are inclusive
{
	//TODO: We need to eventually handle periods in numbers once periods are their own token

	enum LAST_TYPE {NONE, VALUE, OP}; //unop pretends to be VALUE

	struct NODE *root_op_node = NULL;
	struct NODE *current_op_node = NULL;
	struct NODE *left_value_node = NULL;
	struct NODE *previous_node = NULL;

	struct TOKEN *token = start;
	int value_count = 0;
	bool last_was_end = false; //I hate this
	enum LAST_TYPE last_type = NONE;

	while(!last_was_end)
	{
		if(token_is_op(token)) //is an op
		{
			if(token->type == TOKEN_SUB && value_count == 0) //leading minus for negative number
			{
				PARSE_ERROR_LC(token->line_number, token->start_char, "Negative numbers are not currently supported");
				//TODO: Add negative number support
			}
			else
			{
				if(last_type != VALUE)
				{
					PARSE_ERROR_LC(token->line_number, token->start_char, "Expected value but found '%s'", token->string);
				}

				struct NODE *new_node = create_node(AST_OP, token->line_number);
				previous_node = new_node;
				switch(token->type)
				{
					case TOKEN_TEST_EQUAL:
						new_node->op_type = OP_TEST_EQUAL;
						break;

					case TOKEN_ADD:
						new_node->op_type = OP_ADD;
						break;

					case TOKEN_SUB:
						new_node->op_type = OP_SUB;
						break;

					case TOKEN_MUL:
						new_node->op_type = OP_MUL;
						break;

					case TOKEN_DIV:
						new_node->op_type = OP_DIV;
						break;

					default:
						PARSE_ERROR_LC(token->line_number, token->start_char, "Cannot convert operation token to operation");
				}

				if(root_op_node == NULL)
				{
					current_op_node = new_node;
					root_op_node = current_op_node;
					add_node(current_op_node, left_value_node);
				}
				else
				{
					if(get_op_precedence(current_op_node->op_type) >= get_op_precedence(new_node->op_type)) //Same or lower
					{
						struct NODE *old_op_node = current_op_node;
						struct NODE *old_parent = old_op_node->parent;

						if(old_op_node->parent != NULL)
							remove_node(old_op_node->parent, old_op_node);
						add_node(new_node, old_op_node);

						left_value_node = old_op_node;

						if(old_op_node == root_op_node)
						{
							root_op_node = new_node;
						}
						else
						{
							add_node(old_parent, new_node);
						}

						current_op_node = new_node;
					}
					else //Higher
					{
						struct NODE *right_child = current_op_node->last_child;
						remove_node(current_op_node, right_child);
						add_node(new_node, right_child); //Is now the left value of the new op
						assert(left_value_node == NULL);
						left_value_node = right_child;
						add_node(current_op_node, new_node);
						current_op_node = new_node;
					}
				}

				last_type = OP;
			}
		}
		else if(token_is_unop(token)) //Is an unop
		{
			if((left_value_node == NULL && root_op_node == NULL) || !(last_type == VALUE) )
			{
				PARSE_ERROR_LC(token->line_number, token->start_char, "Expected value before unary operation '%s'", token->string);
			}

			struct NODE *new_node = create_node(AST_UNOP, token->line_number);
			switch(token->type)
			{
				case TOKEN_EXCLAMATION:
					new_node->unop_type = UNOP_INVERT;
					break;

				default:
					PARSE_ERROR_LC(token->line_number, token->start_char, "We don't know how to set unop node type from token");
			}

			assert(previous_node != NULL);

			struct NODE *parent = previous_node->parent;
			if(parent != NULL)
				remove_node(parent, previous_node);
			add_node(new_node, previous_node);
			if(parent != NULL)
				add_node(parent, new_node);

			if(previous_node == root_op_node)
				root_op_node = new_node;
			if(previous_node == current_op_node)
				current_op_node = new_node;
			if(previous_node == left_value_node)
				left_value_node = new_node;
			previous_node = new_node;

			last_type = VALUE;
		}
		else //Is a value
		{
			if(last_type == VALUE)
			{
				PARSE_ERROR_LC(token->line_number, token->start_char, "Expected operator but found value '%s'", token->string);
			}

			struct NODE *new_node = NULL;
			if(is_number(token->string))
			{
				new_node = create_node(AST_LITERAL, token->line_number);
				previous_node = new_node;
				free(new_node->type_name);
				new_node->type_name = strdup("Number");
				new_node->literal_type = LITERAL_NUMBER;
				free(new_node->literal_string);
				new_node->literal_string = strdup(token->string);
			}
			else
			{
				if(strcmp(token->string, "true") == 0 || strcmp(token->string, "false") == 0)
				{
					new_node = create_node(AST_LITERAL, token->line_number);
					previous_node = new_node;
					free(new_node->type_name);
					new_node->type_name = strdup("Bool");
					new_node->literal_type = LITERAL_BOOL;
					free(new_node->literal_string);
					new_node->literal_string = strdup(token->string);

					if(strcmp(token->string, "true") == 0)
						new_node->literal_bool = true;
					else if(strcmp(token->string, "false") == 0)
						new_node->literal_bool = false;
					else
						PARSE_ERROR_LC(token->line_number, token->start_char, "Somehow we don't know how to convert this Bool to an AST node");

				}
				else
					PARSE_ERROR_LC(token->line_number, token->start_char, "We don't know how to deal with this non-numerical value");
			}
			assert(new_node != NULL);

			if(left_value_node == NULL)
				left_value_node = new_node;
			else
			{
				add_node(current_op_node, new_node);
				left_value_node = NULL;
			}

			last_type = VALUE;
			value_count++;
		}

		last_was_end = token == end;
		token = token->next;
	}

	if(last_type == OP)
	{
		PARSE_ERROR_LC(token->line_number, token->start_char, "Expected final value following operator '%s'", token_type_name[token->type]);
	}

	if(value_count == 1)
	{
		assert(left_value_node != NULL);
		add_node(root, left_value_node);
	}
	else
	{
		assert(root_op_node != NULL);
		add_node(root, root_op_node);
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
		if(*token == NULL) //We know we reached the end of the file before hitting a semicolon
		{
			PARSE_ERROR("Expected semicolon but found end of file");
		}
	}

	if(start == end && start->type == TOKEN_SEMICOLON)
	{
		PARSE_ERROR_LC(start->line_number, start->start_char, "Expected expression before semicolon");
	}

	parse_expression_bounds(expression_root, start, end);
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
				PARSE_ERROR_LC(token->line_number, token->start_char, "Unexpected close brace");
			}

			current_parse_parent_node = current_parse_parent_node->parent;
			return token->next;
		}
	}

	//At this point we know that we have reached the end of the file
	if(inner_block)
	{
		PARSE_ERROR("Expected close brace but found end of file")
	}
	return NULL;
}

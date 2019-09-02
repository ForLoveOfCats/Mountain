#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "compiler.h"
#include "symbols.h"
#include "ast.h"
#include "parser.h"


#define PARSE_ERROR_LC(line, column, ...) \
	{ \
		printf("Parse error in %s @ line %i column %i: ", paths[current_file], line, column); \
		printf(__VA_ARGS__); \
		printf("\n"); \
		exit(EXIT_FAILURE); \
	} \


#define PARSE_ERROR(...) \
	{ \
		printf("Parse error in %s: ", paths[current_file]); \
		printf(__VA_ARGS__); \
		printf("\n"); \
		exit(EXIT_FAILURE); \
	} \


#define NEXT_TOKEN(token) \
	{ \
		if(token->next == NULL) \
		{ \
			PARSE_ERROR_LC(token->line_number, token->end_char + 1, "Encountered end of file unexpectedly"); \
			exit(EXIT_FAILURE); \
		} \
		token = token->next; \
	} \


char *token_type_name[] = { FOREACH_TOKEN_TYPE(GENERATE_STRING) };


int current_token_start = 0;


void free_token_list(struct TOKEN *token) //Frees a token and all next tokens
{
	struct TOKEN *next = NULL;

	while(token != NULL)
	{
		next = token->next;

		free(token->string);
		free(token);

		token = next;
	}
}


struct TOKEN *next_token_from_file(FILE *source_file)
{
	struct TOKEN *token = malloc(sizeof(struct TOKEN));
	token->type = TOKEN_WORD;
	token->string = strdup("");
	token->next = NULL;

	bool in_token = false;
	int car; //Int so checking for EOF is safe
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

				case ',':
				{
					token->type = TOKEN_COMMA;
					goto return_car_as_token;
				}

				case '#':
				{
					token->type = TOKEN_HASH;
					goto return_car_as_token;
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

				case '>':
				{
					char next_car = fgetc(source_file);
					if(next_car != '=')
					{
						ungetc(next_car, source_file); //HACK FIXME
						token->type = TOKEN_TEST_GREATER;
						goto return_car_as_token;
					}

					token->type = TOKEN_TEST_GREATER_EQUAL;
					goto return_token;
				}

				case '<':
				{
					char next_car = fgetc(source_file);
					if(next_car != '=')
					{
						ungetc(next_car, source_file); //HACK FIXME
						token->type = TOKEN_TEST_LESS;
						goto return_car_as_token;
					}

					token->type = TOKEN_TEST_LESS_EQUAL;
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
					char next_car = fgetc(source_file);
					if(next_car == '/')
					{
						while(car != '\n')
						{
							car = getc(source_file);
						}
						current_file_line++;
						current_file_character = 0;
						break;
					}

					token->type = TOKEN_DIV;
					goto return_car_as_token;
				}

				case '!':
				{
					char next_car = fgetc(source_file);
					if(next_car != '=')
					{
						ungetc(next_car, source_file); //HACK FIXME
						token->type = TOKEN_EXCLAMATION;
						goto return_car_as_token;
					}

					token->type = TOKEN_TEST_NOT_EQUAL;
					goto return_token;
				}

				case '&':
				{
					token->type = TOKEN_ADDRESS_OF;
					goto return_car_as_token;
				}

				case '$':
				{
					token->type = TOKEN_DEREFERENCE;
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
			case ',':
			case '#':
			case '=':
			case '>':
			case '<':
			case '+':
			case '-':
			case '*':
			case '/':
			case '!':
			case '&':
			case '$':
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


struct TOKEN *peek_next_token(struct TOKEN *token)
{
	NEXT_TOKEN(token);
	return token;
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
		case TOKEN_EQUALS:
		case TOKEN_TEST_EQUAL:
		case TOKEN_TEST_NOT_EQUAL:
		case TOKEN_TEST_GREATER:
		case TOKEN_TEST_GREATER_EQUAL:
		case TOKEN_TEST_LESS:
		case TOKEN_TEST_LESS_EQUAL:
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
		case TOKEN_ADDRESS_OF:
		case TOKEN_DEREFERENCE:
			return true;

 		default:
			return false;
	}
}


int get_op_precedence(enum OP_TYPE op)
{
	switch(op)
	{
		case OP_EQUALS:
			return 1;

		case OP_TEST_EQUAL:
		case OP_TEST_NOT_EQUAL:
		case OP_TEST_GREATER:
		case OP_TEST_GREATER_EQUAL:
		case OP_TEST_LESS:
		case OP_TEST_LESS_EQUAL:
			return 2;

		case OP_ADD:
		case OP_SUB:
			return 3;

		case OP_MUL:
		case OP_DIV:
			return 4;

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

				struct NODE *new_node = create_node(AST_OP, current_file, token->line_number, token->start_char, token->end_char);
				free(new_node->literal_string);
				new_node->literal_string = strdup(token->string);
				previous_node = new_node;
				switch(token->type)
				{
					case TOKEN_EQUALS:
						new_node->op_type = OP_EQUALS;
						break;

					case TOKEN_TEST_EQUAL:
						new_node->op_type = OP_TEST_EQUAL;
						break;

					case TOKEN_TEST_NOT_EQUAL:
						new_node->op_type = OP_TEST_NOT_EQUAL;
						break;

					case TOKEN_TEST_GREATER:
						new_node->op_type = OP_TEST_GREATER;
						break;

					case TOKEN_TEST_GREATER_EQUAL:
						new_node->op_type = OP_TEST_GREATER_EQUAL;
						break;

					case TOKEN_TEST_LESS:
						new_node->op_type = OP_TEST_LESS;
						break;

					case TOKEN_TEST_LESS_EQUAL:
						new_node->op_type = OP_TEST_LESS_EQUAL;
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

			struct NODE *new_node = create_node(AST_UNOP, current_file, token->line_number, token->start_char, token->end_char);
			switch(token->type)
			{
				case TOKEN_EXCLAMATION:
					new_node->unop_type = UNOP_INVERT;
					break;

				case TOKEN_ADDRESS_OF:
					new_node->unop_type = UNOP_ADDRESS_OF;
					break;

				case TOKEN_DEREFERENCE:
					new_node->unop_type = UNOP_DEREFERENCE;
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
				new_node = create_node(AST_LITERAL, current_file, token->line_number, token->start_char, token->end_char);
				previous_node = new_node;
				free_type(new_node->type);
				new_node->type = create_type("i32"); //TODO: Test for floating point
				new_node->literal_type = LITERAL_NUMBER;
				free(new_node->literal_string);
				new_node->literal_string = strdup(token->string);
			}
			else
			{
				if(strcmp(token->string, "true") == 0 || strcmp(token->string, "false") == 0)
				{
					new_node = create_node(AST_LITERAL, current_file, token->line_number, token->start_char, token->end_char);
					previous_node = new_node;
					free_type(new_node->type);
					new_node->type = create_type("Bool");
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
				else //Must be a variable get or function call
				{
					if(peek_next_token(token)->type == TOKEN_OPEN_PARENTHESES) //Function call
					{
						new_node = create_node(AST_CALL, current_file, token->line_number, token->start_char, token->end_char);
						free(new_node->name);
						new_node->name = strdup(token->string);

						NEXT_TOKEN(token);
						expect(token, TOKEN_OPEN_PARENTHESES);
						token = parse_function_args(new_node, token);
					}
					else //Variable get
					{
						new_node = create_node(AST_GET, current_file, token->line_number, token->start_char, token->end_char);
						previous_node = new_node;
						free(new_node->name);
						new_node->name = strdup(token->string);
					}
				}
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
		if(!last_was_end)
			token = token->next;
	}

	if(last_type == OP)
	{
		PARSE_ERROR_LC(token->line_number, token->start_char, "Expected final value following operator '%s'", token->string);
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

	token = token->next;
}


struct NODE *parse_expression_to_semicolon(struct TOKEN **token)
{
	struct NODE *expression_root = create_node(AST_EXPRESSION, current_file, (*token)->line_number, (*token)->start_char, (*token)->end_char);

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


//Consumes from open parentheses to close parentheses
struct TOKEN *parse_function_args(struct NODE *func, struct TOKEN *token)
{
	assert(token->type == TOKEN_OPEN_PARENTHESES);
	NEXT_TOKEN(token);

	if(token->type != TOKEN_CLOSE_PARENTHESES)
	{
		while(true) //Loops through each expression
		{
			int open = 0;
			struct TOKEN *expression_start = token;
			struct TOKEN *expression_end = NULL;
			while(true) //Loops through tokens until end of expression
			{
				if(token->type == TOKEN_OPEN_PARENTHESES)
					open++;

				if((token->type == TOKEN_CLOSE_PARENTHESES || token->type == TOKEN_COMMA)
					&& open <= 0) //Reached end
				{
					parse_expression_bounds(func, expression_start, expression_end);

					if(token->type == TOKEN_COMMA)
						NEXT_TOKEN(token); //Skip over the comma

					break; //Continue to next expression
				}

				if(token->type == TOKEN_CLOSE_PARENTHESES)
					open--;

				expression_end = token;
				NEXT_TOKEN(token); //Otherwise move to next token
			}

			if(token->type == TOKEN_CLOSE_PARENTHESES)
				break;
		}
	}

	assert(token->type == TOKEN_CLOSE_PARENTHESES);
	return token;
}


struct TYPE_DATA *parse_type(struct TOKEN **callsite_token)
{
	struct TOKEN *token = *callsite_token;

	expect(token, TOKEN_WORD);

	struct TYPE_DATA *instance = create_type(token->string);
	token = token->next;

	if(token->type == TOKEN_COLON)
	{
		NEXT_TOKEN(token);

		struct TYPE_DATA *parent = parse_type(&token);
		parent->child = instance;

		*callsite_token = token;
		return parent;
	}

	*callsite_token = token;
	return instance;
}


struct TOKEN *parse_next_statement(struct TOKEN *token)
{
	if(token->type == TOKEN_WORD)
	{
		if(strcmp(token->string, "let") == 0)
		{
			struct NODE *new_node = create_node(AST_LET, current_file, token->line_number, token->start_char, token->end_char);

			NEXT_TOKEN(token);
			expect(token, TOKEN_COLON);

			NEXT_TOKEN(token);
			expect(token, TOKEN_WORD);
			free_type(new_node->type);
			new_node->type = parse_type(&token);

			free(new_node->name);
			expect(token, TOKEN_WORD);
			new_node->name = strdup(token->string);

			NEXT_TOKEN(token);
			expect(token, TOKEN_EQUALS);

			NEXT_TOKEN(token);
			if(strcmp(token->string, "undefined") == 0)
			{
				NEXT_TOKEN(token);
				expect(token, TOKEN_SEMICOLON);
			}
			else
				add_node(new_node, parse_expression_to_semicolon(&token));

			add_node(current_parse_parent_node, new_node);

			token = token->next;
		}

		else if(strcmp(token->string, "if") == 0
				|| strcmp(token->string, "elif") == 0)
		{
			enum AST_TYPE node_type = AST_IF;
			if(strcmp(token->string, "elif") == 0)
				node_type = AST_ELIF;
			struct NODE *new_node = create_node(node_type, current_file, token->line_number, token->start_char, token->end_char);

			NEXT_TOKEN(token);
			expect(token, TOKEN_OPEN_PARENTHESES);

			NEXT_TOKEN(token);
			struct TOKEN *start = token;
			struct TOKEN *end = token;
			int open = 0;
			while(true)
			{
				end = token;
				NEXT_TOKEN(token);

				if(token->type == TOKEN_OPEN_PARENTHESES)
					open++;
				else if(token->type == TOKEN_CLOSE_PARENTHESES)
				{
					if(open > 0)
						open--;
					else
						break;
				}
			}

			if(start == end && start->next->type != TOKEN_CLOSE_PARENTHESES)
				PARSE_ERROR_LC(end->line_number, end->start_char, "Expected Bool expresssion but found empty expression");
			struct NODE *expression_root = create_node(AST_EXPRESSION, current_file, start->line_number, start->start_char, start->end_char);
			parse_expression_bounds(expression_root, start, end);
			add_node(new_node, expression_root);

			NEXT_TOKEN(token);
			expect(token, TOKEN_OPEN_BRACE);

			NEXT_TOKEN(token);
			struct NODE *old_parse_parent_node = current_parse_parent_node;
			struct NODE *block = create_node(AST_BLOCK, current_file, token->line_number, token->start_char, token->end_char);
			add_node(new_node, block);
			current_parse_parent_node = block;
			token = parse_block(token, true, 0);
			current_parse_parent_node = old_parse_parent_node;
			add_node(current_parse_parent_node, new_node);
		}

		else if(strcmp(token->string, "else") == 0)
		{
			struct NODE *new_node = create_node(AST_ELSE, current_file, token->line_number, token->start_char, token->end_char);

			NEXT_TOKEN(token);
			expect(token, TOKEN_OPEN_BRACE);

			NEXT_TOKEN(token);
			struct NODE *old_parse_parent_node = current_parse_parent_node;
			struct NODE *block = create_node(AST_BLOCK, current_file, token->line_number, token->start_char, token->end_char);
			add_node(new_node, block);
			current_parse_parent_node = block;
			token = parse_block(token, true, 0);
			current_parse_parent_node = old_parse_parent_node;
			add_node(current_parse_parent_node, new_node);
		}

		else if(strcmp(token->string, "while") == 0) //You guessed it, an while loop
		{
			struct NODE *new_node = create_node(AST_WHILE, current_file, token->line_number, token->start_char, token->end_char);

			NEXT_TOKEN(token);
			expect(token, TOKEN_OPEN_PARENTHESES);

			NEXT_TOKEN(token);
			struct TOKEN *start = token;
			struct TOKEN *end = token;
			while(token->type != TOKEN_CLOSE_PARENTHESES)
			{
				end = token;
				NEXT_TOKEN(token);
			}

			if(start == end && start->next->type != TOKEN_CLOSE_PARENTHESES)
				PARSE_ERROR_LC(end->line_number, end->start_char, "Expected Bool expresssion but found empty expression");
			struct NODE *expression_root = create_node(AST_EXPRESSION, current_file, start->line_number, start->start_char, start->end_char);
			parse_expression_bounds(expression_root, start, end);
			add_node(new_node, expression_root);

			NEXT_TOKEN(token);
			expect(token, TOKEN_OPEN_BRACE);

			NEXT_TOKEN(token);
			struct NODE *old_parse_parent_node = current_parse_parent_node;
			struct NODE *block = create_node(AST_BLOCK, current_file, token->line_number, token->start_char, token->end_char);
			add_node(new_node, block);
			current_parse_parent_node = block;
			token = parse_block(token, true, 0);
			current_parse_parent_node = old_parse_parent_node;
			add_node(current_parse_parent_node, new_node);
		}

		else if(strcmp(token->string, "break") == 0)
		{
			struct NODE *new_node = create_node(AST_BREAK, current_file, token->line_number, token->start_char, token->end_char);

			NEXT_TOKEN(token);
			expect(token, TOKEN_SEMICOLON);

			add_node(current_parse_parent_node, new_node);

			token = token->next;
		}

		else if(strcmp(token->string, "continue") == 0)
		{
			struct NODE *new_node = create_node(AST_CONTINUE, current_file, token->line_number, token->start_char, token->end_char);

			NEXT_TOKEN(token);
			expect(token, TOKEN_SEMICOLON);

			add_node(current_parse_parent_node, new_node);

			token = token->next;
		}

		else if(strcmp(token->string, "return") == 0) //TODO: Allow returning an actual value
		{
			struct NODE *new_node = create_node(AST_RETURN, current_file, token->line_number, token->start_char, token->end_char);

			NEXT_TOKEN(token);
			if(token->type != TOKEN_SEMICOLON)
			{
				struct NODE *expression = parse_expression_to_semicolon(&token);
				add_node(new_node, expression);
			}

			add_node(current_parse_parent_node, new_node);

			token = token->next;
		}

		else if(strcmp(token->string, "func") == 0) //You guessed it, a function declaration
		{
			struct NODE *new_node = create_node(AST_FUNC, current_file, token->line_number, token->start_char, token->end_char);

			NEXT_TOKEN(token);
			expect(token, TOKEN_COLON);

			NEXT_TOKEN(token);
			free_type(new_node->type);
			new_node->type = parse_type(&token);

			expect(token, TOKEN_WORD); //name of the function
			free(new_node->name);
			new_node->name = strdup(token->string);

			NEXT_TOKEN(token);
			expect(token, TOKEN_OPEN_PARENTHESES);

			NEXT_TOKEN(token);
			struct TOKEN *start = token;
			struct TOKEN *end = token;
			while(token->type != TOKEN_CLOSE_PARENTHESES)
			{
				end = token;
				NEXT_TOKEN(token);
			}

			if(start == end && start->next->type != TOKEN_CLOSE_PARENTHESES) //No arguments
			{}
			else //One or more arguments to parse
			{
				struct TOKEN *current_arg_token = start;
				while(true)
				{
					expect(current_arg_token, TOKEN_WORD);
					if(strcmp(current_arg_token->string, "let") != 0)
						PARSE_ERROR_LC(current_arg_token->line_number, current_arg_token->start_char, "Expected function argument declaration to be variable declaration");
					if(current_arg_token == end)
						PARSE_ERROR_LC(current_arg_token->line_number, current_arg_token->start_char, "Expected argument type");

					NEXT_TOKEN(current_arg_token);
					expect(current_arg_token, TOKEN_COLON);
					if(current_arg_token == end)
						PARSE_ERROR_LC(current_arg_token->line_number, current_arg_token->start_char, "Expected argument type");

					NEXT_TOKEN(current_arg_token);
					struct TYPE_DATA *type = parse_type(&current_arg_token);

					if(current_arg_token->type != TOKEN_WORD)
						PARSE_ERROR_LC(current_arg_token->line_number, current_arg_token->start_char, "Expected argument name");
					char *name = current_arg_token->string;

					struct ARG_DATA *arg = create_arg_data(name, type, current_file, token->line_number);
					if(new_node->first_arg == NULL)
					{
						new_node->first_arg = arg;
						new_node->last_arg = arg;
					}
					else
					{
						new_node->last_arg->next = arg;
						new_node->last_arg = arg;
					}

					if(current_arg_token == end)
						break;

					NEXT_TOKEN(current_arg_token);
					expect(current_arg_token, TOKEN_COMMA);
					NEXT_TOKEN(current_arg_token);
				}
			}

			NEXT_TOKEN(token);
			expect(token, TOKEN_OPEN_BRACE);

			NEXT_TOKEN(token);
			struct NODE *old_parse_parent_node = current_parse_parent_node;
			struct NODE *block = create_node(AST_BLOCK, current_file, token->line_number, token->start_char, token->end_char);
			add_node(new_node, block);
			current_parse_parent_node = block;
			token = parse_block(token, true, 0);
			current_parse_parent_node = old_parse_parent_node;

			if(current_parse_parent_node->first_func == NULL)
			{
				current_parse_parent_node->first_func = new_node;
				current_parse_parent_node->last_func = new_node;
			}
			else
			{
				current_parse_parent_node->last_func->next = new_node;
				current_parse_parent_node->last_func = new_node;
			}

			struct FUNC_PROTOTYPE *prototype = create_func_prototype(new_node);
			if(first_func_prototype == NULL)
			{
				first_func_prototype = prototype;
				last_func_prototype = prototype;
			}
			else
			{
				last_func_prototype->next = prototype;
				last_func_prototype = prototype;
			}
		}

		else if(strcmp(token->string, "struct") == 0) //You guessed it, a struct declaration
		{
			NEXT_TOKEN(token);
			expect(token, TOKEN_COLON);

			NEXT_TOKEN(token);
			expect(token, TOKEN_WORD); //Our name
			struct NODE *new_node = create_node(AST_STRUCT, current_file, token->line_number, token->start_char, token->end_char);
			free(new_node->name);
			new_node->name = strdup(token->string);

			NEXT_TOKEN(token);
			expect(token, TOKEN_OPEN_BRACE);
			while(true)
			{
				//TODO: Parse field declarations

				if(token->type == TOKEN_CLOSE_BRACE)
					break;
				NEXT_TOKEN(token);
			}

			add_node(current_parse_parent_node, new_node);

			token = token->next; //Don't check for EOF
		}

		else if(strcmp(token->string, "import") == 0)
		{
			NEXT_TOKEN(token);
			expect(token, TOKEN_WORD);

			struct IMPORT_DATA *import_data = create_import_data(token->string, current_file, token->line_number);
			if(current_parse_parent_node->first_import == NULL)
			{
				current_parse_parent_node->first_import = import_data;
				current_parse_parent_node->last_import = import_data;
			}
			else
			{
				current_parse_parent_node->last_import->next = import_data;
				current_parse_parent_node->last_import = import_data;
			}

			NEXT_TOKEN(token);
			expect(token, TOKEN_SEMICOLON);

			token = token->next; //Don't check for EOF
		}

		else //Not a statement
		{
			add_node(current_parse_parent_node, parse_expression_to_semicolon(&token));
			expect(token, TOKEN_SEMICOLON);
			token = token->next;
		}
	}
	else
	{
		PARSE_ERROR_LC(token->line_number, token->start_char, "Unexpected token '%s' at beginning of new statement", token->string);
	}

	return token;
}


struct TOKEN *parse_block(struct TOKEN *token, bool inner_block, int level)
{
	assert(level >= 0);

	while(true)
	{
		if(token->type == TOKEN_OPEN_BRACE)
		{
			struct NODE *block = create_node(AST_BLOCK, current_file, token->line_number, token->start_char, token->end_char);
			add_node(current_parse_parent_node, block);
			struct NODE *old_parse_parent_node = current_parse_parent_node;
			current_parse_parent_node = block;
			token = parse_block(token->next, true, level + 1);
			current_parse_parent_node = old_parse_parent_node;
		}
		if(token == NULL)
			break;

		if(token->type == TOKEN_CLOSE_BRACE)
		{
			if(!inner_block)
			{
				PARSE_ERROR_LC(token->line_number, token->start_char, "Unexpected close brace");
			}
			return token->next;
		}

		token = parse_next_statement(token);
		if(token == NULL)
			break;

		else if(token->type == TOKEN_CLOSE_BRACE)
		{
			if(!inner_block)
			{
				PARSE_ERROR_LC(token->line_number, token->start_char, "Unexpected close brace");
			}
			return token->next;
		}
	}

	//At this point we know that we have reached the end of the file
	if(inner_block)
	{
		PARSE_ERROR("Expected close brace but found end of file");
	}
	return NULL;
}


void parse_file(FILE *file)
{
	struct TOKEN *first_token = tokenize_file(file);
	if(first_token == NULL)
	{
		return;
	}

	struct TOKEN *token = first_token;

	expect(token, TOKEN_WORD);
	if(strcmp(token->string, "module") != 0)
		PARSE_ERROR_LC(token->line_number, token->start_char, "File must first declare a module");

	NEXT_TOKEN(token);
	expect(token, TOKEN_WORD);

	bool found = false;
	struct NODE *module_root = first_module;
	while(module_root != NULL)
	{
		if(strcmp(module_root->name, token->string) == 0)
		{
			printf("Joining pre-existing module '%s'\n", token->string);
			found = true;
			break;
		}

		module_root = module_root->next;
	}

	if(!found)
	{
		printf("Creating module '%s'\n", token->string);

		module_root = create_node(AST_MODULE, -1, -1, -1, -1);
		free(module_root->name);
		module_root->name = strdup(token->string);

		if(first_module == NULL)
		{
			first_module = module_root;
			last_module = module_root;
		}
		else
		{
			last_module->next = module_root;
			last_module = module_root;
		}
	}

	current_parse_parent_node = module_root;

	NEXT_TOKEN(token);
	expect(token, TOKEN_SEMICOLON);
	token = token->next;

	parse_block(token, false, 0);

	free_token_list(first_token);
}

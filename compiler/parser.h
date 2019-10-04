#pragma once
#include <stdbool.h>
#include <stdio.h>

#include "compiler.h"
#include "ast.h"


#define FOREACH_TOKEN_TYPE(TYPE) \
	TYPE(TOKEN_WORD) \
	TYPE(TOKEN_COLON) \
	TYPE(TOKEN_PERIOD) \
	TYPE(TOKEN_SEMICOLON) \
	TYPE(TOKEN_EQUALS) \
	TYPE(TOKEN_TEST_EQUAL) \
	TYPE(TOKEN_TEST_NOT_EQUAL) \
	TYPE(TOKEN_TEST_GREATER) \
	TYPE(TOKEN_TEST_GREATER_EQUAL) \
	TYPE(TOKEN_TEST_LESS) \
	TYPE(TOKEN_TEST_LESS_EQUAL) \
	TYPE(TOKEN_ADD) \
	TYPE(TOKEN_SUB) \
	TYPE(TOKEN_MUL) \
	TYPE(TOKEN_DIV) \
	TYPE(TOKEN_EXCLAMATION) \
	TYPE(TOKEN_ADDRESS_OF) \
	TYPE(TOKEN_DEREFERENCE) \
	TYPE(TOKEN_OPEN_PARENTHESES) \
	TYPE(TOKEN_CLOSE_PARENTHESES) \
	TYPE(TOKEN_OPEN_BRACE) \
	TYPE(TOKEN_CLOSE_BRACE) \
	TYPE(TOKEN_OPEN_BRACKET) \
	TYPE(TOKEN_CLOSE_BRACKET) \
	TYPE(TOKEN_COMMA) \
	TYPE(TOKEN_HASH) \
	TYPE(TOKEN_U8) \
	TYPE(TOKEN_STRING) \

enum TOKEN_TYPE { FOREACH_TOKEN_TYPE(GENERATE_ENUM) };


struct TOKEN
{
	int line_number;
	int start_char;
	int end_char;

	enum TOKEN_TYPE type;
	char *string;

	struct TOKEN *next;
};


void free_token_list(struct TOKEN *token);


struct TOKEN *tokenize_file(FILE *source_file);


struct TYPE_DATA *parse_type(struct TOKEN **callsite_token, struct TYPE_DATA *child);


struct NODE *parse_expression_to_semicolon(struct TOKEN **token, struct NODE *module);


struct TOKEN *parse_next_statement(struct TOKEN *token);


struct TOKEN *parse_block(struct TOKEN *token, bool inner_block, int level);


void parse_file(FILE *file);


struct TOKEN *parse_function_args(struct NODE *func, struct TOKEN *token);

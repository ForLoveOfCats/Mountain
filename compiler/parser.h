#pragma once
#include <stdbool.h>
#include <stdio.h>

#include "compiler.h"


#define FOREACH_TOKEN_TYPE(TYPE) \
	TYPE(TOKEN_WORD) \
	TYPE(TOKEN_COLON) \
	TYPE(TOKEN_SEMICOLON) \
	TYPE(TOKEN_EQUALS) \
	TYPE(TOKEN_ADD) \
	TYPE(TOKEN_SUB) \
	TYPE(TOKEN_MUL) \
	TYPE(TOKEN_DIV) \
	TYPE(TOKEN_OPEN_PARENTHESES) \
	TYPE(TOKEN_CLOSE_PARENTHESES) \
	TYPE(TOKEN_OPEN_BRACE) \
	TYPE(TOKEN_CLOSE_BRACE) \

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


struct TOKEN *parse_next_statement(struct TOKEN *token);


struct TOKEN *parse_block(struct TOKEN *token, int level);

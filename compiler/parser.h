#pragma once
#include <stdbool.h>
#include <stdio.h>

#include "compiler.h"


#define FOREACH_TOKEN_TYPE(TYPE) \
	TYPE(TOKEN_WORD) \
	TYPE(TOKEN_COLON) \
	TYPE(TOKEN_SEMICOLON) \
	TYPE(TOKEN_COMMENT) \
	TYPE(TOKEN_EQUALS) \
	TYPE(TOKEN_OPEN_PARENTHESES) \
	TYPE(TOKEN_CLOSE_PARENTHESES) \

enum TOKEN_TYPE { FOREACH_TOKEN_TYPE(GENERATE_ENUM) };


struct TOKEN
{
	bool valid;
	enum TOKEN_TYPE type;
	char *string;
};


void free_token(struct TOKEN);


bool parse_next_statement(FILE *source_file);

#pragma once
#include <stdbool.h>
#include <stdio.h>


enum TOKEN_TYPE {TOKEN_WORD, TOKEN_COLON, TOKEN_SEMICOLON, TOKEN_COMMENT, TOKEN_EQUALS, TOKEN_OPEN_PARENTHESES, TOKEN_CLOSE_PARENTHESES};
char *token_type_name[7];

struct TOKEN
{
	bool valid;
	enum TOKEN_TYPE type;
	char *string;
};


void free_token(struct TOKEN);


bool parse_next_statement(FILE *source_file);

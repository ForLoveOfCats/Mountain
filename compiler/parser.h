#pragma once
#include <stdbool.h>
#include <stdio.h>


enum TOKEN_TYPE {TOKEN_WORD, TOKEN_COLON, TOKEN_SEMICOLON, TOKEN_COMMENT};
char *token_type_name[4];

struct TOKEN
{
	bool valid;
	enum TOKEN_TYPE type;
	char *string;
};


bool parse_next_statement(FILE *source_file);

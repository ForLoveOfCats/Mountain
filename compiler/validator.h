#pragma once
#include <stdbool.h>


#include "ast.h"


struct SCOPE
{
	struct SCOPE *parent;

	struct VAR_DATA *first_var;
	struct VAR_DATA *last_var;
};


struct VAR_DATA
{
	struct VAR_DATA *next;

	bool is_mutable;
	char *name;
	int line;
};


struct SCOPE *create_scope(struct SCOPE *parent);


void validate_tree(struct NODE *node);

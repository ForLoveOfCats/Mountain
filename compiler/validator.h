#pragma once
#include <stdbool.h>


#include "ast.h"


struct SCOPE
{
	struct SCOPE *parent;
	struct SCOPE *first_child;
	struct SCOPE *last_child;
	struct SCOPE *next;

	struct VAR_DATA *first_var;
	struct VAR_DATA *last_var;
};


struct VAR_DATA
{
	struct VAR_DATA *next;

	char *name;
	int line;
};


struct SCOPE *create_scope(struct SCOPE *parent);


void free_scope_tree(struct SCOPE *scope);


void validate_block(struct NODE *node, struct SCOPE *scope, int level);

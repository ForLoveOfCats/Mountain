#pragma once
#include <stdbool.h>


#include "ast.h"


bool type_is_number(char *type);


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
	char *type;
	int line;

	int index;
};


bool are_types_equivalent(char *type_one, char *type_two);


struct SCOPE *create_scope(struct SCOPE *parent);


void free_scope_tree(struct SCOPE *scope);


void validate_block(struct NODE *node, struct SCOPE *scope, int level);

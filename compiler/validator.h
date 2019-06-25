#pragma once
#include <stdbool.h>


#include "ast.h"
#include "symbols.h"


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


bool are_types_equivalent(struct TYPE_DATA *type_one, struct TYPE_DATA *type_two);


struct SCOPE *create_scope(struct SCOPE *parent);


void free_scope_tree(struct SCOPE *scope);


void validate_block(struct NODE *node, struct SYMBOL_TABLE *symbol_table, int level);


void populate_function_symbols(struct SYMBOL_TABLE *symbol_table);


void validate_functions(struct SYMBOL_TABLE *symbol_table);

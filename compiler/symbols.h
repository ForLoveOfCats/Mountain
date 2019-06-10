#pragma once
#include <stdbool.h>

#include "compiler.h"



#define FOREACH_SYMBOL_TYPE(TYPE) \
	TYPE(SYMBOL_VAR) \
	TYPE(SYMBOL_STRUCT) \

enum SYMBOL_TYPE { FOREACH_SYMBOL_TYPE(GENERATE_ENUM) };


int next_index;


struct VAR_DATA
{
	char *type;
};


struct SYMBOL
{
	char *name;
	enum SYMBOL_TYPE type;
	int line;

	int index; //Used for name mangling in tranpiled C source

	struct VAR_DATA *var_data;

	struct SYMBOL *next;
};


struct SYMBOL_TABLE
{
	struct SYMBOL_TABLE *parent;
	struct SYMBOL_TABLE *first_child;
	struct SYMBOL_TABLE *last_child;
	struct SYMBOL_TABLE *next;

	struct SYMBOL *first_symbol;
	struct SYMBOL *last_symbol;
};


struct SYMBOL *create_symbol(char *name, enum SYMBOL_TYPE type, int line);


struct SYMBOL_TABLE *create_symbol_table();


void add_table(struct SYMBOL_TABLE *parent, struct SYMBOL_TABLE *child);


void free_table_tree(struct SYMBOL_TABLE *symbol_table);


void add_symbol(struct SYMBOL_TABLE *table, struct SYMBOL *symbol);


struct VAR_DATA *create_var(char *type);


void add_var(struct SYMBOL_TABLE *table, char *name, struct VAR_DATA *var_data, int line);


void free_var(struct VAR_DATA *var);


struct SYMBOL *lookup_symbol(struct SYMBOL_TABLE *table, char *name);

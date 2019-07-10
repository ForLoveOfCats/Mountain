#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "compiler.h"
#include "parser.h"
#include "ast.h"
#include "symbols.h"



struct TYPE_DATA *create_type(char *name)
{
	struct TYPE_DATA *type = malloc(sizeof(struct TYPE_DATA));
	type->name = strdup(name);

	return type;
}


struct TYPE_DATA *copy_type(struct TYPE_DATA *type)
{
	struct TYPE_DATA *new_type = malloc(sizeof(struct TYPE_DATA));
	new_type->name = strdup(type->name);

	return new_type;
}


void free_type(struct TYPE_DATA *type)
{
	free(type->name);
	free(type);
}


struct STRUCT_DATA *create_struct()
{
	struct STRUCT_DATA *struct_data = malloc(sizeof(struct STRUCT_DATA));

	struct_data->index = -1;

	return struct_data;
}


void free_struct(struct STRUCT_DATA *struct_data)
{
	free(struct_data);
}


struct SYMBOL *create_symbol(char *name, enum SYMBOL_TYPE type, int line)
{
	struct SYMBOL *symbol = malloc(sizeof(struct SYMBOL));

	symbol->name = strdup(name);
	symbol->type = type;
	symbol->line = line;

	symbol->index = next_index;
	next_index++;

	symbol->var_data = NULL;
	symbol->struct_data = NULL;

	symbol->next = NULL;

	return symbol;
}


void free_symbol(struct SYMBOL *symbol)
{
	free(symbol->name);

	if(symbol->var_data != NULL)
		free_var(symbol->var_data);

	if(symbol->struct_data != NULL)
		free_struct(symbol->struct_data);

	free(symbol);
}


struct SYMBOL_TABLE *create_symbol_table(struct SYMBOL_TABLE *parent)
{
	struct SYMBOL_TABLE *table = malloc(sizeof(struct SYMBOL_TABLE));

	table->parent = NULL;
 	table->first_child = NULL;
	table->last_child = NULL;
	table->next = NULL;

	table->first_symbol = NULL;
	table->last_symbol = NULL;

	if(parent != NULL)
		add_table(parent, table);

	return table;
}


void add_table(struct SYMBOL_TABLE *parent, struct SYMBOL_TABLE *child)
{
	child->parent = parent;

	if(parent->first_child == NULL)
	{
		parent->first_child = child;
		parent->last_child = child;
	}
	else
	{
		parent->last_child->next = child;
		parent->last_child = child;
	}
}


void free_table_tree(struct SYMBOL_TABLE *symbol_table)
{
	struct SYMBOL_TABLE *child = symbol_table->first_child;
	while(child != NULL)
	{
		struct SYMBOL_TABLE *next = child->next;
		free_table_tree(child);
		child = next;
	}

	struct SYMBOL *symbol = symbol_table->first_symbol;
	while(symbol != NULL)
	{
		struct SYMBOL *next = symbol->next;
		free_symbol(symbol);
		symbol = next;
	}
	free(symbol_table);
}


void add_symbol(struct SYMBOL_TABLE *table, struct SYMBOL *symbol)
{
	if(table->first_symbol == NULL)
	{
		table->first_symbol = symbol;
		table->last_symbol = symbol;
	}
	else
	{
		table->last_symbol->next = symbol;
		table->last_symbol = symbol;
	}
}


struct VAR_DATA *create_var(struct TYPE_DATA *type)
{
	struct VAR_DATA *var_data = malloc(sizeof(struct VAR_DATA));
	var_data->type = type;
	return var_data;
}


void add_var(struct SYMBOL_TABLE *table, char *name, struct VAR_DATA *var_data, int line)
{
	struct SYMBOL *symbol = create_symbol(name, SYMBOL_VAR, line);
	symbol->var_data = var_data;
	add_symbol(table, symbol);
}


void free_var(struct VAR_DATA *var)
{
	free(var);
}


struct SYMBOL *lookup_symbol(struct SYMBOL_TABLE *table, char *name)
{
	struct SYMBOL *symbol = table->first_symbol;
	while(symbol != NULL)
	{
		if(strcmp(symbol->name, name) == 0)
			return symbol;

		symbol = symbol->next;
	}

	if(table->parent != NULL)
		return lookup_symbol(table->parent, name);
	else
		return NULL;
}
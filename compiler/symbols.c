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

	type->index = -1;
	type->name = strdup(name);
	type->reach_module = NULL;
	type->child = NULL;

	return type;
}


struct TYPE_DATA *copy_type(struct TYPE_DATA *type)
{
	struct TYPE_DATA *new_type = create_type(type->name);
	if(type->reach_module != NULL)
		new_type->reach_module = strdup(type->reach_module);

	if(type->child != NULL)
		new_type->child = copy_type(type->child);

	return new_type;
}


void free_type(struct TYPE_DATA *type)
{
	free(type->name);
	free(type->reach_module);

	if(type->child != NULL)
		free_type(type->child);

	free(type);
}


struct ENUM_DATA *create_enum(struct NODE *node)
{
	struct ENUM_DATA *enum_data = malloc(sizeof(struct ENUM_DATA));

	enum_data->node = node;

	return enum_data;
}


void free_enum(struct ENUM_DATA *enum_data)
{
	free(enum_data);
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


struct SYMBOL *create_symbol(char *name, enum SYMBOL_TYPE type, int file, int line)
{
	struct SYMBOL *symbol = malloc(sizeof(struct SYMBOL));

	symbol->name = strdup(name);
	symbol->type = type;
	symbol->file = file;
	symbol->line = line;

	symbol->index = next_index;
	next_index++;

	symbol->var_data = NULL;
	symbol->enum_data = NULL;
	symbol->struct_data = NULL;
	symbol->func_data = NULL;

	symbol->next = NULL;

	return symbol;
}


void free_symbol(struct SYMBOL *symbol)
{
	free(symbol->name);

	if(symbol->var_data != NULL)
		free_var(symbol->var_data);

	if(symbol->enum_data != NULL)
		free_enum(symbol->enum_data);

	if(symbol->struct_data != NULL)
		free_struct(symbol->struct_data);

	if(symbol->func_data != NULL)
		free_func(symbol->func_data);

	free(symbol);
}


struct SYMBOL_TABLE *create_symbol_table(struct SYMBOL_TABLE *parent, struct NODE *module)
{
	struct SYMBOL_TABLE *table = malloc(sizeof(struct SYMBOL_TABLE));

	table->module = module;

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


void add_var(struct SYMBOL_TABLE *table, char *name, struct VAR_DATA *var_data, int file, int line)
{
	struct SYMBOL *symbol = create_symbol(name, SYMBOL_VAR, file, line);
	symbol->var_data = var_data;
	add_symbol(table, symbol);
}


void free_var(struct VAR_DATA *var)
{
	free(var);
}


struct FUNC_DATA *create_func(struct TYPE_DATA *return_type)
{
	struct FUNC_DATA *func_data = malloc(sizeof(struct FUNC_DATA));

	func_data->first_arg = NULL;
	func_data->last_arg = NULL;
	func_data->return_type = return_type;

	return func_data;
}


void free_func(struct FUNC_DATA *func_data)
{
	free(func_data); //The contents are freed by their original owners
}


struct SYMBOL *lookup_symbol(struct SYMBOL_TABLE *table, char *name, int file, bool search_using_imports)
{
	struct SYMBOL *symbol = table->first_symbol;
	while(symbol != NULL)
	{
		if(strcmp(symbol->name, name) == 0)
			return symbol;

		symbol = symbol->next;
	}

	if(table->parent != NULL)
		return lookup_symbol(table->parent, name, file, search_using_imports);

	if(search_using_imports)
	{
		struct IMPORT_DATA *import_data = table->module->first_import;
		while(import_data != NULL)
		{
			if(import_data->is_using)
			{
				struct NODE *module = first_module;
				while(module != NULL)
				{
					if(strcmp(module->name, import_data->name) == 0 && import_data->file == file)
					{
						struct SYMBOL *returned = lookup_symbol(module->symbol_table, name, file, false);
						if(returned != NULL)
							return returned;
					}

					module = module->next;
				}
			}

			import_data = import_data->next;
		}
	}

	return NULL;
}

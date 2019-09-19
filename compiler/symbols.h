#pragma once
#include <stdbool.h>

#include "compiler.h"



#define TypeVoid -1
#define TypeBool -2
#define Typei32 -3
#define Typeu8 -4
#define TypePtr -5



#define FOREACH_SYMBOL_TYPE(TYPE) \
	TYPE(SYMBOL_VAR) \
	TYPE(SYMBOL_ENUM) \
	TYPE(SYMBOL_STRUCT) \
	TYPE(SYMBOL_FUNC) \

enum SYMBOL_TYPE { FOREACH_SYMBOL_TYPE(GENERATE_ENUM) };


int next_index;


struct TYPE_DATA //TODO: Add type paremeters
{
	int index;
	int type;
	char *name;
	char *reach_module;
	struct TYPE_DATA *child;
};


struct ENUM_DATA //Used for any type definition
{
	struct NODE *node;
};


struct STRUCT_DATA //Used for any type definition
{
	int index;
};


struct VAR_DATA
{
	struct TYPE_DATA *type;
};


struct ARG_DATA
{
	struct ARG_DATA *next;

	char *name;
	struct TYPE_DATA *type;
	int file;
	int line_number;

	int index;
};


struct FUNC_DATA
{
	struct ARG_DATA *first_arg;
	struct ARG_DATA *last_arg;

	struct TYPE_DATA *return_type;
};


struct SYMBOL
{
	char *name;
	enum SYMBOL_TYPE type;
	int file;
	int line;

	int index; //Used for name mangling in tranpiled C source

	struct VAR_DATA *var_data;
	struct ENUM_DATA *enum_data;
	struct STRUCT_DATA *struct_data;
	struct FUNC_DATA *func_data;

	struct SYMBOL *next;
};


struct SYMBOL_TABLE
{
	struct NODE *module;

	struct SYMBOL_TABLE *parent;
	struct SYMBOL_TABLE *first_child;
	struct SYMBOL_TABLE *last_child;
	struct SYMBOL_TABLE *next;

	struct SYMBOL *first_symbol;
	struct SYMBOL *last_symbol;
};


struct TYPE_DATA *create_type(char *name);


struct TYPE_DATA *copy_type(struct TYPE_DATA *type);


void free_type(struct TYPE_DATA *type);


struct ENUM_DATA *create_enum(struct NODE *node);


void free_enum(struct ENUM_DATA *enum_data);


struct STRUCT_DATA *create_struct();


void free_struct(struct STRUCT_DATA *struct_data);


struct SYMBOL *create_symbol(char *name, enum SYMBOL_TYPE type, int file, int line);


struct SYMBOL_TABLE *create_symbol_table(struct SYMBOL_TABLE *parent, struct NODE *module);


void add_table(struct SYMBOL_TABLE *parent, struct SYMBOL_TABLE *child);


void free_table_tree(struct SYMBOL_TABLE *symbol_table);


void add_symbol(struct SYMBOL_TABLE *table, struct SYMBOL *symbol);


struct VAR_DATA *create_var(struct TYPE_DATA *type);


void add_var(struct SYMBOL_TABLE *table, char *name, struct VAR_DATA *var_data, int file, int line);


void free_var(struct VAR_DATA *var);


struct FUNC_DATA *create_func(struct TYPE_DATA *return_type);


void free_func(struct FUNC_DATA *func_data);


struct SYMBOL *lookup_symbol(struct SYMBOL_TABLE *table, char *name, int file, bool search_using_imports);

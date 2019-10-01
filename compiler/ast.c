#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "compiler.h"
#include "symbols.h"
#include "parser.h"
#include "ast.h"



struct NODE *lookup_module(char *name)
{
	struct NODE *module = first_module;
	while(module != NULL)
	{
		if(strcmp(module->name, name) == 0)
			return module;

		module = module->next;
	}

	return NULL;
}


struct IMPORT_DATA *create_import_data(char *name, bool is_using, int file, int line_number)
{
	struct IMPORT_DATA *new_import_data = malloc(sizeof(struct IMPORT_DATA));

	new_import_data->name = strdup(name);
	new_import_data->is_using = is_using;

	new_import_data->file = file;
	new_import_data->line_number = line_number;

	new_import_data->next = NULL;

	return new_import_data;
}


void free_import_data(struct IMPORT_DATA *import_data)
{
	free(import_data->name);
	free(import_data);
}


struct NODE *create_node(enum AST_TYPE type, struct NODE *module, int file, int line_number, int start_char, int end_char)
{
	struct NODE *new_node = (struct NODE *)malloc(sizeof(struct NODE));

	new_node->node_type = type;

	new_node->module = module;
	new_node->parent = NULL;
	new_node->next = NULL;
	new_node->previous = NULL;
	new_node->first_child = NULL;
	new_node->last_child = NULL;

	new_node->first_import = NULL;
	new_node->last_import = NULL;

	new_node->first_arg = NULL;
	new_node->last_arg = NULL;

	new_node->file = file;
	new_node->line_number = line_number;
	new_node->start_char = start_char;
	new_node->end_char = end_char;

	new_node->symbol_table = NULL;
	new_node->index = -1;

	new_node->type = create_type("");
	new_node->name = strdup("");

	new_node->literal_string = strdup("");
	new_node->literal_bool = false;

	new_node->op_type = OP_NONE;
	new_node->unop_type = UNOP_NONE;

	return new_node;
}


void add_node(struct NODE *parent, struct NODE *new_node)
{
	new_node->parent = parent;
	new_node->next = NULL;

	if(parent->first_child == NULL)
	{
		parent->first_child = new_node;
		parent->last_child = new_node;
	}
	else
	{
		parent->last_child->next = new_node;
		new_node->previous = parent->last_child;
		parent->last_child = new_node;
	}
}


void remove_node(struct NODE *parent, struct NODE *child)
{
	if(parent->last_child == child)
		parent->last_child = child->previous;

	if(child->previous == NULL) //We are the first node
	{
		parent->first_child = child->next; //This is fine if next is NULL
		if(child->next != NULL)
			child->next->previous = NULL;
	}
	else //We are *not* the first node
	{
		child->previous->next = child->next; //This is fine if next is NULL
		if(child->next != NULL)
			child->next->previous = child->previous;
	}

	child->parent = NULL;
}


int count_node_children(struct NODE *node)
{
	int count = 0;

	struct NODE *child = node->first_child;
	while(child != NULL)
	{
		count++;
		child = child->next;
	}

	return count;
}


int recursive_count_node_children(struct NODE *node)
{
	int count = count_node_children(node);

	struct NODE *child = node->first_child;
	while(child != NULL)
	{
		count += recursive_count_node_children(child);
		child = child->next;
	}

	return count;
}


void free_tree(struct NODE *node) //Frees *node and all descendant nodes
{
	struct NODE *child = node->first_child;
	while(child != NULL)
	{
		struct NODE *next_child = child->next;
		free_tree(child);
		child = next_child;
	}

	struct ARG_DATA *arg = node->first_arg;
	while(arg != NULL)
	{
		struct ARG_DATA *next_arg = arg->next;
		free(arg->name);
		free_type(arg->type);
		free(arg);
		arg = next_arg;
	}

	struct IMPORT_DATA *import_data = node->first_import;
	struct IMPORT_DATA *next_import_data = NULL;
	while(import_data != NULL)
	{
		next_import_data = import_data->next;
		free_import_data(import_data);
		import_data = next_import_data;
	}

	free_type(node->type);
	free(node->name);
	free(node->literal_string);
	free(node);
}


struct ARG_DATA *create_arg_data(char *name, struct TYPE_DATA *type, int file, int line_number)
{
	struct ARG_DATA *arg = malloc(sizeof(struct ARG_DATA));

	arg->next = NULL;

	arg->name = strdup(name);
	arg->type = type;
	arg->file = file;
	arg->line_number = line_number;

	arg->index = -1;

	return arg;
}


struct FUNC_PROTOTYPE *create_func_prototype(struct NODE *func)
{
	assert(func->node_type == AST_FUNC);

	struct FUNC_PROTOTYPE *instance = malloc(sizeof(struct FUNC_PROTOTYPE));
	instance->func = func;
	instance->next = NULL;

	return instance;
}


void free_func_prototype_list(struct FUNC_PROTOTYPE *func)
{
	//The actual AST node for the func will be freed while freeing the tree
	//so all we need to do is free the prototype's themselves

	struct FUNC_PROTOTYPE *next_func = NULL;
	while(func != NULL)
	{
		next_func = func->next;
		free(func);
		func = next_func;
	}
}

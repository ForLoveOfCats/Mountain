#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "compiler.h"
#include "symbols.h"
#include "parser.h"
#include "ast.h"


struct NODE *create_node(enum AST_TYPE type, int line_number, int start_char, int end_char)
{
	struct NODE *new_node = (struct NODE *)malloc(sizeof(struct NODE));

	new_node->type = type;

	new_node->parent = NULL;
	new_node->next = NULL;
	new_node->previous = NULL;
	new_node->first_child = NULL;
	new_node->last_child = NULL;

	new_node->first_arg = NULL;
	new_node->last_arg = NULL;

	new_node->line_number = line_number;
	new_node->start_char = start_char;
	new_node->end_char = end_char;

	new_node->symbol_table = NULL;
	new_node->index = -1;

	new_node->type_name = create_type("");
	new_node->variable_name = strdup("");
	new_node->function_name = strdup("");

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
		free(arg->type);
		free(arg);
		arg = next_arg;
	}

	free_type(node->type_name);
	free(node->variable_name);
	free(node->function_name);
	free(node->literal_string);
	free(node);
}


struct ARG_DATA *create_arg_data(char *name, char *type)
{
	struct ARG_DATA *arg = malloc(sizeof(struct ARG_DATA));

	arg->next = NULL;

	arg->name = strdup(name);
	arg->type = strdup(type);

	arg->index = -1;

	return arg;
}

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "compiler.h"
#include "ast.h"


struct NODE *create_node(enum AST_TYPE type)
{
	struct NODE *new_node = (struct NODE *)malloc(sizeof(struct NODE));

	new_node->type = type;

	new_node->parent = NULL;
	new_node->next_node = NULL;
	new_node->first_child = NULL;
	new_node->last_child = NULL;

	new_node->type_name = strdup("");

	new_node->stack_len = 0;
	new_node->def_location = 0;

	new_node->def_name = strdup("");

	return new_node;
}


void add_node(struct NODE *new_node)
{
	new_node->parent = current_parse_parent_node;
	new_node->next_node = NULL;
	new_node->first_child = NULL;
	new_node->last_child = NULL;

	if(current_parse_parent_node->first_child == NULL)
	{
		current_parse_parent_node->first_child = new_node;
		current_parse_parent_node->last_child = new_node;
	}
	else
	{
		current_parse_parent_node->last_child->next_node = new_node;
		current_parse_parent_node->last_child = new_node;
	}
}


void free_node(struct NODE *node) //Frees *node and all descendant nodes
{
	struct NODE *child;
	child = node->first_child;
	while(child != NULL)
	{
		struct NODE *new_child = child->next_node;
		free_node(child);
		child = new_child;
	}

	free(node->type_name);
	free(node->def_name);
	free(node);
}

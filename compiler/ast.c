#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "compiler.h"
#include "parser.h"
#include "ast.h"


struct NODE *create_node(enum AST_TYPE type, int line_number)
{
	struct NODE *new_node = (struct NODE *)malloc(sizeof(struct NODE));

	new_node->type = type;

	new_node->parent = NULL;
	new_node->next = NULL;
	new_node->first_child = NULL;
	new_node->last_child = NULL;

	new_node->line_number = line_number;

	new_node->index = -1;

	new_node->type_name = strdup("");

	new_node->variable_name = strdup("");

	new_node->literal_string = strdup("");

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
		parent->last_child = new_node;
	}
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


void free_tree(struct NODE *node) //Frees *node and all descendant nodes
{
	struct NODE *child = node->first_child;
	while(child != NULL)
	{
		struct NODE *next_child = child->next;
		free_tree(child);
		child = next_child;
	}

	free(node->type_name);
	free(node->variable_name);
	free(node->literal_string);
	free(node);
}

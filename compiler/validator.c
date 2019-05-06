#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ast.h"
#include "validator.h"


int count;


struct VAR_DATA *add_var(struct SCOPE *scope, char *name, char *type, int line)
{
	struct VAR_DATA *var = malloc(sizeof(struct VAR_DATA));
	var->next = NULL;
	var->name = strdup(name);
	var->type = strdup(type);
	var->line = line;

	var->index = count;
	count++;

	if(scope->first_var == NULL)
	{
		scope->first_var = var;
		scope->last_var = var;
		return var;
	}

	scope->last_var->next = var;
	scope->last_var = var;
	return var;
}


struct VAR_DATA *get_var(struct SCOPE *scope, char *name)
{
	struct VAR_DATA *var = scope->first_var;
	while(var != NULL)
	{
		if(strcmp(var->name, name) == 0)
			return var;

		var = var->next;
	}

	if(scope->parent != NULL)
		return get_var(scope->parent, name);

	return NULL;
}


bool var_exists(struct SCOPE *scope, char *name)
{
	return get_var(scope, name) != NULL;
}


void free_var(struct VAR_DATA *var) //futureproofing
{
	free(var->name);
	free(var->type);
	free(var);
}


void free_var_list(struct VAR_DATA *var)
{
	while(var != NULL)
	{
		struct VAR_DATA *next = var->next;
		free_var(var);
		var = next;
	}
}


struct SCOPE *create_scope(struct SCOPE *parent)
{
	struct SCOPE *scope = malloc(sizeof(struct SCOPE));
	scope->parent = parent;
	scope->first_child = NULL;
	scope->last_child = NULL;
	scope->next = NULL;
	scope->first_var = NULL;
	scope->last_var = NULL;

	if(parent != NULL)
	{
		if(parent->first_child == NULL)
		{
			parent->first_child = scope;
			parent->last_child = scope;
		}
		else
		{
			parent->last_child->next = scope;
			parent->last_child = scope;
		}
	}

	return scope;
}


void free_scope_tree(struct SCOPE *scope)
{
	if(scope->first_child != NULL)
	{
		struct SCOPE *child = scope->first_child;
		while(child != NULL)
		{
			struct SCOPE *next = child->next;
			free_scope_tree(child);
			child = next;
		}
	}

	free_var_list(scope->first_var);
	free(scope);
}


void validate_block(struct NODE *node, struct SCOPE *scope, int level) //Exits on failure
{
	assert(node->type == AST_BLOCK);
	node = node->first_child;

	assert(level >= 0);
	if(level == 0)
		count = 0;

	//"foreach" node
	while(node != NULL)
	{
		switch(node->type)
		{
			case AST_BLOCK:
			{
				validate_block(node, create_scope(scope), level + 1);
				break;
			}

			case AST_VAR:
			{
				assert(count_node_children(node) == 1);

				if(var_exists(scope, node->variable_name))
				{
					printf("Validation error @ line %i: Variable '%s' already exists from line %i\n",
					       node->line_number, node->variable_name, get_var(scope, node->variable_name)->line);
					exit(EXIT_FAILURE);
				}
				struct VAR_DATA *var = add_var(scope, node->variable_name, node->type_name, node->line_number);
				node->index = var->index;

				if(strcmp(node->type_name, "Int32") != 0) //TODO allow non-int values
				{
					printf("Validation error @ line %i: Variable defined with non-Int32 type\n", node->line_number);
					exit(EXIT_FAILURE);
				}

				break;
			}

			case AST_SET:
			{
				assert(count_node_children(node) == 1);

				struct VAR_DATA *var = get_var(scope, node->variable_name);

				if(var == NULL)
				{
					printf("Validation error @ line %i: Cannot set variable '%s' as it does not exist\n", node->line_number, node->variable_name);
					exit(EXIT_FAILURE);
				}

				if(strcmp(var->type, node->first_child->type_name) != 0)
				{
					printf("Validation error @ line %i: Cannot set variable '%s' due to type mismatch\n", node->line_number, node->variable_name);
					printf("Var type: '%s'     Set type: '%s'\n", var->type, node->first_child->type_name);
					exit(EXIT_FAILURE);
				}

				node->index = var->index;
			}

			default: //To get ccls to shut up
				break;
		}

		node = node->next;
	}
}

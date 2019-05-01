#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ast.h"
#include "validator.h"


void add_var(struct SCOPE *scope, char *name, bool is_mutable, int line)
{
	struct VAR_DATA *var = malloc(sizeof(struct VAR_DATA));
	var->next = NULL;
	var->is_mutable = is_mutable;
	var->name = strdup(name);
	var->line = line;

	if(scope->first_var == NULL)
	{
		scope->first_var = var;
		scope->last_var = var;
		return;
	}

	scope->last_var->next = var;
	scope->last_var = var;
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
	return NULL;
}


bool var_exists(struct SCOPE *scope, char *name)
{
	return get_var(scope, name) != NULL;
}


void free_var(struct VAR_DATA *var) //futureproofing
{
	free(var->name);
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
	scope->first_var = NULL;
	scope->last_var = NULL;
	return scope;
}


void free_scope(struct SCOPE *scope) //futureproofing
{
	free_var_list(scope->first_var);
	free(scope);
}


void validate_tree(struct NODE *node) //Exits on failure
{
	assert(node->type == AST_ROOT);
	node = node->first_child;

	struct SCOPE *root_scope = create_scope(NULL);
	struct SCOPE *scope = root_scope;

	//"foreach" node
	while(node != NULL)
	{
		switch(node->type)
		{
			case AST_DEF:
			{
				assert(count_node_children(node) == 1);

				if(var_exists(scope, node->variable_name))
				{
					printf("Validation Error: Variable '%s' on line %i already exists from line %i\n",
					       node->variable_name, node->line_number, get_var(scope, node->variable_name)->line);
					exit(EXIT_FAILURE);
				}
				add_var(scope, node->variable_name, false, node->line_number);

				if(strcmp(node->type_name, "Int") != 0) //TODO allow non-int values
				{
					printf("Validation Error: Variable defined with non-Int type on line %i\n", node->line_number);
					exit(EXIT_FAILURE);
				}

				break;
			}

			case AST_MUT:
			{
				assert(count_node_children(node) == 1);

				if(var_exists(scope, node->variable_name))
				{
					printf("Validation Error: Variable '%s' on line %i already exists from line %i\n",
					       node->variable_name, node->line_number, get_var(scope, node->variable_name)->line);
					exit(EXIT_FAILURE);
				}
				add_var(scope, node->variable_name, true, node->line_number);

				if(strcmp(node->type_name, "Int") != 0) //TODO allow non-int values
				{
					printf("Validation Error: Variable defined with non-Int type on line %i\n", node->line_number);
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

				if(!var->is_mutable)
				{
					printf("Validation error @ line %i: Cannot set to immutable variable '%s'\n", node->line_number, node->variable_name);
					exit(EXIT_FAILURE);
				}
			}

			default: //To get ccls to shut up
				break;
		}

		node = node->next_node;
	}

	free_scope(root_scope);
}

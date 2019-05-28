#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ast.h"
#include "validator.h"


//TODO: Add column information to message
#define VALIDATE_ERROR_L(line, ...) \
	{ \
		printf("Validation error @ line %i: ", line); \
		printf(__VA_ARGS__); \
		printf("\n"); \
		exit(EXIT_FAILURE); \
	} \


int count;


bool type_is_number(char *type)
{
	if(strcmp(type, "Number") == 0)
		return true;

	else if(strcmp(type, "Int32") == 0)
		return true;

	return false;
}


bool are_types_equivalent(char *type_one, char *type_two)
{
	if(strcmp(type_one, type_two) == 0)
		return true;

	if(type_is_number(type_one) && type_is_number(type_two))
		return true;

	return false;
}


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


char *typecheck_expression(struct NODE *node, struct SCOPE *scope, int level) //Exits on failure
{
	if(node->type == AST_EXPRESSION)
	{
		return typecheck_expression(node->first_child, scope, level + 1);
	}

	int child_count = count_node_children(node);
	assert(child_count <= 2 && child_count >= 0);

	if(child_count == 0)
	{
		if(node->type == AST_GET)
		{
			if(!var_exists(scope, node->variable_name))
				VALIDATE_ERROR_L(node->line_number, "Cannot get variable '%s' as it does not exist", node->variable_name);

			struct VAR_DATA *var = get_var(scope, node->variable_name);
			node->index = var->index;
			return var->type;
		}
		else
			return node->type_name;
	}
	else if(child_count == 1)
	{
		assert(node->type == AST_UNOP);

		char *type = typecheck_expression(node->first_child, scope, level + 1);

		switch(node->unop_type)
		{
			case UNOP_INVERT:
			{
				if(strcmp(type, "Bool") != 0)
					VALIDATE_ERROR_L(node->line_number, "Cannot invert non-boolean value of type '%s'", type);
				break;
			}

			default:
				VALIDATE_ERROR_L(node->line_number, "We don't know how to typecheck this unop");
		}

		return type;
	}
	else if(child_count == 2)
	{
		assert(node->type == AST_OP);

		char *left_type = typecheck_expression(node->first_child, scope, level + 1);
		char *right_type = typecheck_expression(node->last_child, scope, level + 1);

		if(!are_types_equivalent(left_type, right_type))
			VALIDATE_ERROR_L(node->line_number, "Type mismatch between left and right values for operation in expression");

		//TODO: Use a switch statement here
		if(node->op_type == OP_TEST_EQUAL)
			return "Bool";
		else if(node->op_type == OP_TEST_NOT_EQUAL)
			return "Bool";
		else if(node->op_type == OP_TEST_GREATER)
			return "Bool";
		else if(node->op_type == OP_TEST_GREATER_EQUAL)
			return "Bool";
		else if(node->op_type == OP_TEST_LESS)
			return "Bool";
		else if(node->op_type == OP_TEST_LESS_EQUAL)
			return "Bool";
		else
		{
			if(node->type == AST_OP && (!type_is_number(left_type) || !type_is_number(right_type)) )
				VALIDATE_ERROR_L(node->line_number, "Cannot perform arithmetic on one or more non-numerical values");
			return left_type;
		}
	}
	else
	{
		printf("INTERNAL ERROR: Unsupported child count of expression subnode\n");
		exit(EXIT_FAILURE);
	}
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
					VALIDATE_ERROR_L(node->line_number, "Variable '%s' already exists from line %i", node->variable_name, get_var(scope, node->variable_name)->line);

				struct VAR_DATA *var = add_var(scope, node->variable_name, node->type_name, node->line_number);
				node->index = var->index;

				char *expression_type = typecheck_expression(node->first_child, scope, 0);
				if(!are_types_equivalent(expression_type, node->type_name))
				{
					VALIDATE_ERROR_L(node->line_number, "Type mismatch declaring variable '%s' of type '%s' with expression of type '%s'",
									 node->variable_name, node->type_name, expression_type);
				}

				break;
			}

			case AST_SET:
			{
				assert(count_node_children(node) == 1);

				struct VAR_DATA *var = get_var(scope, node->variable_name);

				if(var == NULL)
					VALIDATE_ERROR_L(node->line_number, "Cannot set variable '%s' as it does not exist", node->variable_name);

				char *expression_type = typecheck_expression(node->first_child, scope, 0);
				if(!are_types_equivalent(expression_type, var->type))
				{
					VALIDATE_ERROR_L(node->line_number, "Type mismatch setting variable '%s' of type '%s' with expression of type '%s'",
									 node->variable_name, var->type, expression_type);
				}

				node->index = var->index;
				break;
			}

			case AST_IF:
			{
				assert(count_node_children(node) == 2);

				char *expression_type = typecheck_expression(node->first_child, scope, 0);
				if(strcmp(expression_type, "Bool") != 0)
					VALIDATE_ERROR_L(node->line_number, "Expected a Bool expression but found expression of type '%s'", expression_type);

				validate_block(node->last_child, scope, level + 1);

				break;
			}

			default:
				printf("INTERNAL ERROR: We don't know how to validate this statement\n");
				exit(EXIT_FAILURE);
		}

		node = node->next;
	}
}

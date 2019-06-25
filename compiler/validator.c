#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ast.h"
#include "symbols.h"
#include "validator.h"


//TODO: Add column information to message
#define VALIDATE_ERROR_L(line, ...) \
	{ \
		printf("Validation error @ line %i: ", line); \
		printf(__VA_ARGS__); \
		printf("\n"); \
		exit(EXIT_FAILURE); \
	} \


bool type_is_number(char *type)
{
	if(strcmp(type, "Number") == 0)
		return true;

	else if(strcmp(type, "i32") == 0)
		return true;

	return false;
}


bool are_types_equivalent(struct TYPE_DATA *type_one, struct TYPE_DATA *type_two)
{
	if(strcmp(type_one->name, type_two->name) == 0)
		return true;

	if(type_is_number(type_one->name) && type_is_number(type_two->name))
		return true;

	return false;
}


struct TYPE_DATA *typecheck_expression(struct NODE *node, struct SYMBOL_TABLE *symbol_table, int level) //Exits on failure
{
	if(node->type == AST_EXPRESSION)
	{
		return typecheck_expression(node->first_child, symbol_table, level + 1);
	}

	int child_count = count_node_children(node);
	assert(child_count <= 2 && child_count >= 0);

	if(child_count == 0)
	{
		if(node->type == AST_GET)
		{
			struct SYMBOL *symbol = lookup_symbol(symbol_table, node->variable_name);
			if(symbol == NULL || symbol->type != SYMBOL_VAR)
				VALIDATE_ERROR_L(node->line_number, "Cannot get variable '%s' as it does not exist", node->variable_name);

			node->index = symbol->index;
			return copy_type(symbol->var_data->type);
		}
		else
			return copy_type(node->type_name);
	}
	else if(child_count == 1)
	{
		assert(node->type == AST_UNOP);

		struct TYPE_DATA *type = typecheck_expression(node->first_child, symbol_table, level + 1);

		switch(node->unop_type)
		{
			case UNOP_INVERT:
			{
				if(strcmp(type->name, "Bool") != 0)
					VALIDATE_ERROR_L(node->line_number, "Cannot invert non-boolean value of type '%s'", type->name);
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

		struct TYPE_DATA *left_type = typecheck_expression(node->first_child, symbol_table, level + 1);
		struct TYPE_DATA *right_type = typecheck_expression(node->last_child, symbol_table, level + 1);

		if(!are_types_equivalent(left_type, right_type))
			VALIDATE_ERROR_L(node->line_number, "Type mismatch between left and right values for operation in expression");

		if(node->op_type == OP_TEST_EQUAL
		   || node->op_type == OP_TEST_NOT_EQUAL
		   || node->op_type == OP_TEST_GREATER
		   || node->op_type == OP_TEST_GREATER_EQUAL
		   || node->op_type == OP_TEST_LESS
		   || node->op_type == OP_TEST_LESS_EQUAL)
		{
			free_type(left_type);
			free_type(right_type);
			return create_type("Bool");
		}
		else
		{
			if(node->type == AST_OP && (!type_is_number(left_type->name) || !type_is_number(right_type->name)) )
				VALIDATE_ERROR_L(node->line_number, "Cannot perform arithmetic on one or more non-numerical values");

			free_type(right_type);
			return left_type;
		}
	}
	else
	{
		printf("INTERNAL ERROR: Unsupported child count of expression subnode\n");
		exit(EXIT_FAILURE);
	}
}


void validate_block(struct NODE *node, struct SYMBOL_TABLE *symbol_table, int level) //Exits on failure
{
	assert(node->type == AST_BLOCK);
	assert(node->symbol_table == NULL);
	node->symbol_table = symbol_table;
	node = node->first_child;

	assert(level >= 0);

	//"foreach" node
	while(node != NULL)
	{
		switch(node->type)
		{
			case AST_BLOCK:
			{
				validate_block(node, create_symbol_table(symbol_table), level + 1);
				break;
			}

			case AST_VAR: //TODO: Make sure that type is valid and known
			{
				assert(count_node_children(node) == 1);

				{
					struct SYMBOL *symbol = lookup_symbol(symbol_table, node->variable_name);
					if(symbol != NULL && symbol->type == SYMBOL_VAR)
						VALIDATE_ERROR_L(node->line_number, "Variable '%s' already exists from line %i", node->variable_name, symbol->line);
				}

				node->index = next_index; //Not increasing as add_var will do that when creating the symbol
				struct VAR_DATA *var = create_var(node->type_name);
				add_var(symbol_table, node->variable_name, var, node->line_number);

				struct TYPE_DATA *expression_type = typecheck_expression(node->first_child, symbol_table, 0);
				if(!are_types_equivalent(expression_type, node->type_name))
				{
					VALIDATE_ERROR_L(node->line_number, "Type mismatch declaring variable '%s' of type '%s' with expression of type '%s'",
									 node->variable_name, node->type_name->name, expression_type->name);
				}
				free_type(expression_type);

				break;
			}

			case AST_SET:
			{
				assert(count_node_children(node) == 1);

				struct SYMBOL *symbol = lookup_symbol(symbol_table, node->variable_name);

				if(symbol == NULL || symbol->type != SYMBOL_VAR)
					VALIDATE_ERROR_L(node->line_number, "Cannot set variable '%s' as it does not exist", node->variable_name);

				struct TYPE_DATA *expression_type = typecheck_expression(node->first_child, symbol_table, 0);
				if(!are_types_equivalent(expression_type, symbol->var_data->type))
				{
					VALIDATE_ERROR_L(node->line_number, "Type mismatch setting variable '%s' of type '%s' with expression of type '%s'",
									 node->variable_name, symbol->var_data->type->name, expression_type->name);
				}
				free_type(expression_type);

				node->index = symbol->index;
				break;
			}

			case AST_IF:
			{
				assert(count_node_children(node) == 2);

				struct TYPE_DATA *expression_type = typecheck_expression(node->first_child, symbol_table, 0);
				if(strcmp(expression_type->name, "Bool") != 0)
					VALIDATE_ERROR_L(node->line_number, "Expected a Bool expression but found expression of type '%s'", expression_type->name);
				free_type(expression_type);

				validate_block(node->last_child, symbol_table, level + 1);

				break;
			}

			case AST_WHILE:
			{
				assert(count_node_children(node) == 2);

				struct TYPE_DATA *expression_type = typecheck_expression(node->first_child, symbol_table, 0);
				if(strcmp(expression_type->name, "Bool") != 0)
					VALIDATE_ERROR_L(node->line_number, "Expected a Bool expression but found expression of type '%s'", expression_type->name);
				free_type(expression_type);

				validate_block(node->last_child, symbol_table, level + 1);

				break;
			}

			case AST_STRUCT:
			{
				struct SYMBOL *symbol = create_symbol(node->type_name->name, SYMBOL_STRUCT, node->line_number);
				symbol->struct_data = create_struct();
				symbol->struct_data->index = next_index;
				next_index++;
				add_symbol(symbol_table, symbol);

				break;
			}

			default:
				printf("INTERNAL ERROR: We don't know how to validate this statement\n");
				exit(EXIT_FAILURE);
		}

		node = node->next;
	}
}


void populate_function_symbols(struct SYMBOL_TABLE *symbol_table)
{
	struct NODE *node = first_function;
	while(node != NULL)
	{
		add_symbol(symbol_table, create_symbol(node->function_name, SYMBOL_FUNC, node->line_number));
		node = node->next;
	}
}


void validate_functions(struct SYMBOL_TABLE *symbol_table)
{
	struct NODE *node = first_function;
	while(node != NULL)
	{
		assert(node->type == AST_FUNC);
		assert(node->first_child->type == AST_BLOCK);
		assert(count_node_children(node) == 1);

		struct ARG_DATA *arg = node->first_arg;
		while (arg != NULL) // TODO: Validate types as well
		{
			arg->index = next_index;
			next_index++;

			arg = arg->next;
		}

		validate_block(node->first_child, symbol_table, 0);

		node->index = next_index;
		next_index++;

		node = node->next;
	}
}

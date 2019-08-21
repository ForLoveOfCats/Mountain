#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "compiler.h"
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


struct TYPE_DATA *typecheck_expression(struct NODE *node, struct SYMBOL_TABLE *symbol_table, bool global, int level) //Exits on failure
{
	if(node->node_type == AST_EXPRESSION)
	{
		return typecheck_expression(node->first_child, symbol_table, global, level + 1);
	}
	else if(node->node_type == AST_LITERAL)
	{
		return copy_type(node->type);
	}
	else if(node->node_type == AST_GET)
	{
		if(global)
			VALIDATE_ERROR_L(node->line_number, "Cannot get variable contents in root scope");

		struct SYMBOL *symbol = lookup_symbol(symbol_table, node->name);
		if(symbol == NULL || symbol->type != SYMBOL_VAR)
			VALIDATE_ERROR_L(node->line_number, "Cannot get variable '%s' as it does not exist", node->name);

		node->index = symbol->index;
		return copy_type(symbol->var_data->type);
	}
	else if(node->node_type == AST_CALL)
	{
		if(global)
			VALIDATE_ERROR_L(node->line_number, "Cannot call function in root scope");

		struct SYMBOL *function = lookup_symbol(symbol_table, node->name);
		if(function == NULL || function->type != SYMBOL_FUNC)
			VALIDATE_ERROR_L(node->line_number, "No function named '%s'", node->name);

		struct ARG_DATA *func_arg = function->func_data->first_arg;
		struct NODE *call_arg = node->first_child; //The call args are expression nodes
		while(func_arg != NULL)
		{
			if(call_arg == NULL)
				VALIDATE_ERROR_L(node->line_number, "To few arguments when calling function '%s'", function->name);

			struct TYPE_DATA *call_arg_type = typecheck_expression(call_arg, symbol_table, global, level + 1);
			if(!are_types_equivalent(func_arg->type, call_arg_type))
				VALIDATE_ERROR_L(node->line_number, "Type mismatch on parameter '%s'", func_arg->name);
			free_type(call_arg_type);

			func_arg = func_arg->next;
			call_arg = call_arg->next;
		}
		if(call_arg != NULL)
			VALIDATE_ERROR_L(node->line_number, "To many arguments when calling function '%s'", function->name);

		node->index = function->index;
		return copy_type(function->func_data->return_type);
	}
	else if(node->node_type == AST_UNOP)
	{
		struct TYPE_DATA *type = typecheck_expression(node->first_child, symbol_table, global, level + 1);

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
	else if(node->node_type == AST_OP)
	{
		struct TYPE_DATA *left_type = typecheck_expression(node->first_child, symbol_table, global, level + 1);
		struct TYPE_DATA *right_type = typecheck_expression(node->last_child, symbol_table, global, level + 1);

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
			if(node->node_type == AST_OP && (!type_is_number(left_type->name) || !type_is_number(right_type->name)) )
				VALIDATE_ERROR_L(node->line_number, "Cannot perform arithmetic on one or more non-numerical values");

			free_type(right_type);
			return left_type;
		}
	}
	else
	{
		printf("INTERNAL ERROR: Unsupported expression subnode type %i\n", node->node_type);
		exit(EXIT_FAILURE);
	}
}


void validate_block(struct NODE *node, struct SYMBOL_TABLE *symbol_table, bool root, int level) //Exits on failure
{
	assert(node->node_type == AST_BLOCK);
	assert(node->symbol_table == NULL);
	assert(level >= 0);
	node->symbol_table = symbol_table;

	struct NODE *block = node;
	populate_function_symbols(symbol_table, block);

	//"foreach" node
	node = node->first_child;
	while(node != NULL)
	{
		//If in root scope disallow execution
		if(root)
		{
			switch(node->node_type)
			{
				case AST_VAR:
				case AST_FUNC:
				case AST_STRUCT:
					break;

				default: //TODO: Improve error message
					VALIDATE_ERROR_L(node->line_number, "Disallowed in root scope");
					break;
			}
		}

		//Validate the node
		switch(node->node_type)
		{
			case AST_BLOCK:
			{
				validate_block(node, create_symbol_table(symbol_table), false, level + 1);
				break;
			}

			case AST_VAR: //TODO: Make sure that type is valid and known
			{
				int child_count = count_node_children(node);
				assert(child_count == 0 || child_count == 1);

				{
					struct SYMBOL *symbol = lookup_symbol(symbol_table, node->name);
					if(symbol != NULL && symbol->type == SYMBOL_VAR)
						VALIDATE_ERROR_L(node->line_number, "Variable '%s' already exists from line %i", node->name, symbol->line);
				}

				node->index = next_index; //Not increasing as add_var will do that when creating the symbol
				struct VAR_DATA *var = create_var(node->type);
				add_var(symbol_table, node->name, var, node->line_number);

				if(strcmp(node->type->name, "Void") == 0)
					VALIDATE_ERROR_L(node->line_number, "Invalid type 'Void' when declaring variable '%s'", node->name);

				if(child_count == 1)
				{
					struct TYPE_DATA *expression_type = typecheck_expression(node->first_child, symbol_table, root, 0);
					if(!are_types_equivalent(expression_type, node->type))
					{
						VALIDATE_ERROR_L(node->line_number, "Type mismatch declaring variable '%s' of type '%s' with expression of type '%s'",
										 node->name, node->type->name, expression_type->name);
					}
					free_type(expression_type);
				}

				break;
			}

			case AST_SET:
			{
				assert(count_node_children(node) == 1);

				struct SYMBOL *symbol = lookup_symbol(symbol_table, node->name);

				if(symbol == NULL || symbol->type != SYMBOL_VAR)
					VALIDATE_ERROR_L(node->line_number, "Cannot set variable '%s' as it does not exist", node->name);

				struct TYPE_DATA *expression_type = typecheck_expression(node->first_child, symbol_table, root, 0);
				if(!are_types_equivalent(expression_type, symbol->var_data->type))
				{
					VALIDATE_ERROR_L(node->line_number, "Type mismatch setting variable '%s' of type '%s' with expression of type '%s'",
									 node->name, symbol->var_data->type->name, expression_type->name);
				}
				free_type(expression_type);

				node->index = symbol->index;
				break;
			}

			case AST_IF:
			case AST_ELIF:
			{
				assert(count_node_children(node) == 2);

				if(node->node_type == AST_ELIF)
				{
					if(node->previous->node_type != AST_IF && node->previous->node_type != AST_ELIF)
						VALIDATE_ERROR_L(node->line_number, "An 'elif' statement must follow an 'if' or 'elif'");
				}

				struct TYPE_DATA *expression_type = typecheck_expression(node->first_child, symbol_table, root, 0);
				if(strcmp(expression_type->name, "Bool") != 0)
					VALIDATE_ERROR_L(node->line_number, "Expected a Bool expression but found expression of type '%s'", expression_type->name);
				free_type(expression_type);

				validate_block(node->last_child, create_symbol_table(symbol_table), false, level + 1);

				break;
			}

			case AST_ELSE:
			{
				assert(count_node_children(node) == 1);

				if(node->previous->node_type != AST_IF && node->previous->node_type != AST_ELIF)
					VALIDATE_ERROR_L(node->line_number, "An 'else' statement must follow an 'if' or 'elif'");

				validate_block(node->last_child, create_symbol_table(symbol_table), false, level + 1);

				break;
			}

			case AST_WHILE:
			{
				assert(count_node_children(node) == 2);

				struct TYPE_DATA *expression_type = typecheck_expression(node->first_child, symbol_table, root, 0);
				if(strcmp(expression_type->name, "Bool") != 0)
					VALIDATE_ERROR_L(node->line_number, "Expected a Bool expression but found expression of type '%s'", expression_type->name);
				free_type(expression_type);

				validate_block(node->last_child, symbol_table, false, level + 1);

				break;
			}

			case AST_BREAK:
			{
				assert(count_node_children(node) == 0);

				struct NODE *parent = node->parent;
				while(true)
				{
					if(parent->node_type == AST_WHILE)
						break;

					parent = parent->parent;
					if(parent == NULL)
						break;
				}
				if(parent == NULL)
					VALIDATE_ERROR_L(node->line_number, "Cannot break, no loop found");

				break;
			}

			case AST_CONTINUE:
			{
				assert(count_node_children(node) == 0);

				struct NODE *parent = node->parent;
				while(true)
				{
					if(parent->node_type == AST_WHILE)
						break;

					parent = parent->parent;
					if(parent == NULL)
						break;
				}
				if(parent == NULL)
					VALIDATE_ERROR_L(node->line_number, "Cannot continue, no loop found");

				break;
			}

			case AST_RETURN:
			{
				struct NODE *parent = node->parent;
				while(true)
				{
					if(parent->node_type == AST_FUNC)
						break;

					parent = parent->parent;
					if(parent == NULL)
						break;
				}
				if(parent == NULL)
					VALIDATE_ERROR_L(node->line_number, "Cannot return, not in a function");

				assert(parent->node_type == AST_FUNC);
				if(strcmp(parent->type->name, "Void") != 0 && count_node_children(node) == 0)
				{
					VALIDATE_ERROR_L(node->line_number, "Must return a value of type '%s'", parent->type->name);
				}
				else if(count_node_children(node) == 1)
				{
					struct TYPE_DATA *type = typecheck_expression(node->first_child, symbol_table, root, level + 1);
					if(!are_types_equivalent(type, parent->type))
						VALIDATE_ERROR_L(node->line_number, "Type mismatch returning a value of type '%s' from a function of type '%s'",
						                 type->name, parent->type->name);
					free_type(type);
				}

				break;
			}

			case AST_CALL:
			{
				struct SYMBOL *function = lookup_symbol(symbol_table, node->name);
				if(function == NULL || function->type != SYMBOL_FUNC)
					VALIDATE_ERROR_L(node->line_number, "No function named '%s'", node->name);

				struct ARG_DATA *func_arg = function->func_data->first_arg;
				struct NODE *call_arg = node->first_child; //The call args are expression nodes
				while(func_arg != NULL)
				{
					if(call_arg == NULL)
						VALIDATE_ERROR_L(node->line_number, "To few arguments when calling function '%s'", function->name);

					struct TYPE_DATA *call_arg_type = typecheck_expression(call_arg, symbol_table, root, level + 1);
					if(!are_types_equivalent(func_arg->type, call_arg_type))
						VALIDATE_ERROR_L(node->line_number, "Type mismatch on parameter '%s'", func_arg->name);
					free_type(call_arg_type);

					func_arg = func_arg->next;
					call_arg = call_arg->next;
				}
				if(call_arg != NULL)
					VALIDATE_ERROR_L(node->line_number, "To many arguments when calling function '%s'", function->name);

				node->index = function->index;
				break;
			}

			case AST_STRUCT:
			{
				struct SYMBOL *symbol = create_symbol(node->name, SYMBOL_STRUCT, node->line_number);
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

	validate_functions(block);
}


void populate_function_symbols(struct SYMBOL_TABLE *symbol_table, struct NODE *block)
{
	struct NODE *node = block->first_func;
	while(node != NULL)
	{
		struct SYMBOL *symbol = create_symbol(node->name, SYMBOL_FUNC, node->line_number);

		struct FUNC_DATA *func_data = create_func(node->type);
		func_data->first_arg = node->first_arg;
		func_data->last_arg = node->last_arg;
		symbol->func_data = func_data;

		node->index = next_index;
		symbol->index = next_index;
		next_index++;

		add_symbol(symbol_table, symbol);
		node = node->next;
	}
}


bool is_return_satisfied(struct NODE *block)
{
	bool satisfied = false;
	bool if_else_satisfied = false;

	struct NODE *cursor = block->first_child;
	while(cursor != NULL)
	{
		if(cursor->node_type == AST_RETURN)
		{
			satisfied = true;
			break;
		}

		if(cursor->node_type == AST_IF)
			if_else_satisfied = is_return_satisfied(cursor->first_child->next);
		else if(cursor->node_type == AST_ELIF && if_else_satisfied)
			if_else_satisfied = is_return_satisfied(cursor->first_child->next);
		else if(cursor->node_type == AST_ELSE && if_else_satisfied)
		{
			if(is_return_satisfied(cursor->first_child))
			{
				satisfied = true;
				break;
			}
		}

		cursor = cursor->next;
	}

	return satisfied;
}


void trace_function_return(struct NODE *func)
{
	assert(func->node_type == AST_FUNC);
	struct NODE *block = func->first_child;
	assert(block->node_type == AST_BLOCK);

	if(!is_return_satisfied(block))
		VALIDATE_ERROR_L(func->line_number, "Not all code paths return a value");
}


void validate_functions(struct NODE *block)
{
	struct NODE *node = block->first_func;
	while(node != NULL)
	{
		assert(node->node_type == AST_FUNC);
		assert(node->first_child->node_type == AST_BLOCK);
		assert(count_node_children(node) == 1);

		struct SYMBOL_TABLE *symbol_table = create_symbol_table(root_symbol_table);

		struct ARG_DATA *arg = node->first_arg;
		while (arg != NULL) // TODO: Validate types as well
		{
			arg->index = next_index; //next_index is incremented in add_var
			add_var(symbol_table, arg->name, create_var(arg->type), arg->line_number);

			arg = arg->next;
		}

		validate_block(node->first_child, symbol_table, false, 0);
		if(strcmp(node->type->name, "Void") != 0)
			trace_function_return(node);

		node = node->next;
	}
}

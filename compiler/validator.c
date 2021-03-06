#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <execinfo.h>

#include "compiler.h"
#include "ast.h"
#include "symbols.h"
#include "validator.h"


//TODO: Add column information to message
#define VALIDATE_ERROR_LF(line, file, ...) \
	{ \
		printf("Validation error in %s @ line %i: ", paths[file], line); \
		printf(__VA_ARGS__); \
		printf("\n"); \
		exit(EXIT_FAILURE); \
	} \

#define VALIDATE_ERROR(...) \
	{ \
		printf("Validation error: "); \
		printf(__VA_ARGS__); \
		printf("\n"); \
		exit(EXIT_FAILURE); \
	} \


//NOTE: Only call when about to exit(EXIT_FAILURE)
char *fatal_pretty_type_name(struct TYPE_DATA *type)
{
	if(type->child != NULL)
	{
		char *child_name = fatal_pretty_type_name(type->child);
		int child_name_len = strlen(child_name);
		int name_len = strlen(type->name);

		char *output = malloc(child_name_len + name_len + 2);
		strcpy(output, child_name);
		output[child_name_len] = ':';
		output[child_name_len + 1] = '\0';
		strcat(output, type->name);
		output[child_name_len + name_len + 1] = '\0';

		free(child_name);
		return output;
	}
	else
		return type->name;
}


bool is_lvalue(struct NODE *node)
{
	if(node->node_type == AST_GET
	   || (node->node_type == AST_UNOP && node->unop_type == UNOP_DEREFERENCE)
	   || node->node_type == AST_FIELDGET)
		return true;

	if(node->node_type == AST_NAME)
	{
		while(true)
		{
			node = node->first_child;

			if(node == NULL)
				return false;

			if(node->node_type == AST_GET
			   || (node->node_type == AST_UNOP && node->unop_type == UNOP_DEREFERENCE)
			   || node->node_type == AST_FIELDGET)
				return true;
		}
	}

	return false;
}


bool type_is_number(struct TYPE_DATA *type)
{
	assert(type->type != 0);

	switch(type->type)
	{
		case Typeu8:
		case Typei32:
		case TypePtr:
			return true;

		default:
			return false;
	}
}


bool are_types_equal(struct TYPE_DATA *type_one, struct TYPE_DATA *type_two)
{
	assert(type_one->type != 0 && type_two->type != 0);

	if(type_one->type != type_two->type)
		return false;

	if(type_one->child == NULL && type_two->child == NULL)
	  return true;
	else
	{
		//We know that at least one child is not null
		//So it stands to reason that if either is null then !=
		if(type_one->child == NULL || type_two->child == NULL)
		  return false;

		//Both children are not null

		if(type_one->type == TypePtr && type_two->type == TypePtr)
		{
			if(type_one->child->type == TypeVoid || type_two->child->type == TypeVoid)
				return true; //A Void:Ptr equals type with T:Ptr of any type T
		}

		return are_types_equal(type_one->child, type_two->child); //Make sure that the children types are equal
	}
}


void verify_type_valid(struct TYPE_DATA *type, struct SYMBOL_TABLE *symbol_table, bool allow_void, int line, int file)
{
	if(type->type != 0) //We've already run
		return; //We know it's valid

	#define disallow_child_type_and_return \
	{ \
		if(type->child != NULL) \
			VALIDATE_ERROR_LF(line, file, "Type '%s' cannot have any child types", type->name); \
		return;\
	} \

	if(strcmp(type->name, "Void") == 0)
	{
		type->type = TypeVoid;

		if(allow_void)
			disallow_child_type_and_return;

		VALIDATE_ERROR_LF(line, file, "Invalid type 'Void'");
	}

	else if(strcmp(type->name, "i32") == 0)
	{
		type->type = Typei32;
		disallow_child_type_and_return;
	}

	else if(strcmp(type->name, "Bool") == 0)
	{
		type->type = TypeBool;
		disallow_child_type_and_return;
	}

	else if(strcmp(type->name, "u8") == 0)
	{
		type->type = Typeu8;
		disallow_child_type_and_return;
	}

	else if(strcmp(type->name, "Ptr") == 0)
	{
		type->type = TypePtr;

		if(type->child == NULL)
			VALIDATE_ERROR_LF(line, file, "Pointer must have child type");

		verify_type_valid(type->child, symbol_table, true, line, file);
		return;
	}

	struct SYMBOL *symbol = NULL;

	if(type->reach_module != NULL)
	{
		if(strcmp(type->reach_module, symbol_table->module->name) == 0)
			symbol = lookup_symbol(symbol_table->module->symbol_table, type->name, file, true);

		else
		{
			bool found = false;
			struct IMPORT_DATA *import_data = symbol_table->module->first_import;
			while(import_data != NULL)
			{
				if(strcmp(import_data->name, type->reach_module) == 0)
				{
					struct NODE *module = lookup_module(type->reach_module);
					assert(module != NULL);
					found = true;

					symbol = lookup_symbol(module->symbol_table, type->name, file, true);

					break;
				}

				import_data = import_data->next;
			}

			if(!found)
				VALIDATE_ERROR_LF(line, file, "No module has been imported with the name '%s'", type->reach_module);
		}

	}
	else
		symbol = lookup_symbol(symbol_table, type->name, file, true);

	if(symbol == NULL)
		VALIDATE_ERROR_LF(line, file, "Unknown type '%s'", fatal_pretty_type_name(type));

	if(symbol->type == SYMBOL_ENUM)
	{
		type->index = symbol->enum_data->node->index;
		type->type = type->index;
		disallow_child_type_and_return;
	}

	else if(symbol->type == SYMBOL_STRUCT)
	{
		type->index = symbol->struct_data->node->index;
		type->type = type->index;
		disallow_child_type_and_return;
	}

	printf("INTERNAL ERROR: We don't know how to verify type '%s'\n", fatal_pretty_type_name(type));
	exit(EXIT_FAILURE);
}


struct TYPE_DATA *typecheck_field_get(struct NODE *get, struct SYMBOL *struct_symbol)
{
	assert(get->node_type == AST_GET || get->node_type == AST_FIELDGET);
	assert(struct_symbol->type == SYMBOL_STRUCT);

	get->node_type = AST_FIELDGET;

	struct NODE *field = struct_symbol->struct_data->node->first_child->first_child;
	while(field != NULL)
	{
		assert(field->node_type == AST_LET);

		if(strcmp(get->name, field->name) == 0)
		{
			get->index = field->index;
			return copy_type(field->type);
		}

		field = field->next;
	}

	VALIDATE_ERROR_LF(get->line_number, get->file, "Struct '%s' contains no field called '%s'",
	                  struct_symbol->struct_data->node->name, get->name);
}


struct TYPE_DATA *typecheck_expression(struct NODE *node, struct SYMBOL_TABLE *symbol_table, bool global, bool search_using_imports, int level) //Exits on failure
{
	if(node->node_type == AST_EXPRESSION)
	{
		return typecheck_expression(node->first_child, symbol_table, global, search_using_imports, level + 1);
	}
	else if(node->node_type == AST_LITERAL)
	{
		verify_type_valid(node->type, symbol_table, false, node->line_number, node->file);
		return copy_type(node->type);
	}
	else if(node->node_type == AST_NAME)
	{
		struct SYMBOL *symbol = lookup_symbol(symbol_table, node->name, node->file, true);
		if(symbol != NULL && symbol->type == SYMBOL_ENUM)
		{
			assert(node->first_child != NULL && node->first_child->node_type == AST_GET);

			bool found = false;
			struct NODE *entry = symbol->enum_data->node->first_child;
			while(entry != NULL)
			{
				assert(entry->node_type == AST_NAME);

				if(strcmp(node->first_child->name, entry->name) == 0)
				{
					found = true;
					node->first_child->index = entry->index;
					break;
				}

				entry = entry->next;
			}

			if(!found)
				VALIDATE_ERROR_LF(node->line_number, node->file, "The enum '%s' contains no entry '%s'",
				                  node->name, node->first_child->name)

			verify_type_valid(symbol->enum_data->node->type, symbol_table, false, node->line_number, node->file);
			return copy_type(symbol->enum_data->node->type);
		}
		else if(symbol != NULL && symbol->type == SYMBOL_VAR)
		{
			struct SYMBOL *struct_symbol = NULL;
			if(symbol->var_data->type->reach_module != NULL)
			{
				struct NODE *reach_module = lookup_module(symbol->var_data->type->reach_module);
				if(reach_module == NULL)
					VALIDATE_ERROR_LF(node->line_number, node->file, "No module has been imported with the name '%s'", symbol->var_data->type->reach_module);
				struct_symbol = lookup_symbol(reach_module->symbol_table, symbol->var_data->type->name, node->file, false);
			}
			else
				struct_symbol = lookup_symbol(symbol_table, symbol->var_data->type->name, node->file, true);

			if(struct_symbol != NULL)
			{
				node->index = symbol->index;
				return typecheck_field_get(node->first_child, struct_symbol);
			}
		}

		if(strcmp(node->name, node->module->name) == 0)
			return typecheck_expression(node->first_child, node->module->symbol_table, global, false, level + 1);

		struct IMPORT_DATA *import_data = node->module->first_import;
		while(import_data != NULL)
		{
			if(strcmp(import_data->name, node->name) == 0)
			{
				struct NODE *module = lookup_module(node->name);
				assert(module != NULL);

				if(import_data->file == node->file)
					return typecheck_expression(node->first_child, module->symbol_table, global, false, level + 1);
			}

			import_data = import_data->next;
		}

		VALIDATE_ERROR_LF(node->line_number, node->file, "No situationally valid symbol '%s'", node->name);
	}
	else if(node->node_type == AST_GET)
	{
		if(global)
			VALIDATE_ERROR_LF(node->line_number, node->file, "Cannot get variable contents in root scope");

		struct SYMBOL *symbol = lookup_symbol(symbol_table, node->name, node->file, search_using_imports);
		if(symbol == NULL || symbol->type != SYMBOL_VAR)
			VALIDATE_ERROR_LF(node->line_number, node->file, "Cannot get variable '%s' as it does not exist", node->name);

		node->index = symbol->index;
		return copy_type(symbol->var_data->type);
	}
	else if(node->node_type == AST_CALL)
	{
		if(global)
			VALIDATE_ERROR_LF(node->line_number, node->file, "Cannot call function in root scope");

		struct SYMBOL *function = lookup_symbol(symbol_table, node->name, node->file, search_using_imports);
		if(function == NULL || function->type != SYMBOL_FUNC)
			VALIDATE_ERROR_LF(node->line_number, node->file, "No function named '%s'", node->name);

		struct ARG_DATA *func_arg = function->func_data->first_arg;
		struct NODE *call_arg = node->first_child; //The call args are expression nodes
		while(func_arg != NULL)
		{
			if(call_arg == NULL)
				VALIDATE_ERROR_LF(node->line_number, node->file, "To few arguments when calling function '%s'", function->name);

			struct TYPE_DATA *call_arg_type = typecheck_expression(call_arg, symbol_table, global, search_using_imports, level + 1);
			if(!are_types_equal(func_arg->type, call_arg_type))
				VALIDATE_ERROR_LF(node->line_number, node->file, "Type mismatch on parameter '%s'", func_arg->name);
			free_type(call_arg_type);

			func_arg = func_arg->next;
			call_arg = call_arg->next;
		}
		if(call_arg != NULL)
			VALIDATE_ERROR_LF(node->line_number, node->file, "To many arguments when calling function '%s'", function->name);

		node->index = function->index;
		return copy_type(function->func_data->return_type);
	}
	else if(node->node_type == AST_FIELDGET)
	{
		struct TYPE_DATA *child_type = typecheck_expression(node->first_child, symbol_table, global, search_using_imports, level + 1);

		struct SYMBOL *symbol = NULL;
		if(child_type->reach_module != NULL)
		{
			struct NODE *reach_module = lookup_module(child_type->reach_module);
			if(reach_module == NULL)
				VALIDATE_ERROR_LF(node->line_number, node->file, "No module has been imported with the name '%s'", child_type->reach_module);
			symbol = lookup_symbol(reach_module->symbol_table, child_type->name, node->file, false);
		}
		else
			symbol = lookup_symbol(symbol_table, child_type->name, node->file, true);

		if(symbol == NULL)
		{
			VALIDATE_ERROR_LF(node->line_number, node->file, "Type '%s' is not known in this context", node->type->name);
		}
		else if(symbol->type != SYMBOL_STRUCT)
			VALIDATE_ERROR_LF(node->line_number, node->file, "Type '%s' has no fields", node->type->name);

		free_type(child_type);

		return typecheck_field_get(node, symbol);
	}
	else if(node->node_type == AST_UNOP)
	{
		struct TYPE_DATA *child_type = typecheck_expression(node->first_child, symbol_table, global, search_using_imports, level + 1);

		switch(node->unop_type)
		{
			case UNOP_INVERT:
			{
				if(strcmp(child_type->name, "Bool") != 0)
					VALIDATE_ERROR_LF(node->line_number, node->file, "Cannot invert non-boolean value of type '%s'", child_type->name);
				break;
			}

			case UNOP_ADDRESS_OF:
			{
				if(!is_lvalue(node->first_child))
					VALIDATE_ERROR_LF(node->line_number, node->file, "Can only take the address of an lvalue");

				struct TYPE_DATA *type = create_type("Ptr");
				type->type = TypePtr;
				type->child = child_type;
				return type;
			}

			case UNOP_DEREFERENCE:
			{
				if(strcmp(child_type->name, "Ptr") != 0)
					VALIDATE_ERROR_LF(node->line_number, node->file, "Cannot dereference a non-pointer value");

				assert(child_type->child != NULL); //if a pointer doesn't have a child type then we did an oopsie

				struct TYPE_DATA *type = copy_type(child_type->child);
				free_type(child_type);
				return type;
			}

			default:
				VALIDATE_ERROR_LF(node->line_number, node->file, "We don't know how to typecheck this unop");
		}

		return child_type;
	}
	else if(node->node_type == AST_OP)
	{
		struct TYPE_DATA *left_type = typecheck_expression(node->first_child, symbol_table, global, search_using_imports, level + 1);
		verify_type_valid(left_type, symbol_table, false, node->first_child->line_number, node->first_child->file);

		struct TYPE_DATA *right_type = typecheck_expression(node->last_child, symbol_table, global, search_using_imports, level + 1);
		verify_type_valid(right_type, symbol_table, false, node->last_child->line_number, node->last_child->file);

		if(!are_types_equal(left_type, right_type))
			VALIDATE_ERROR_LF(node->line_number, node->file,
			                 "Type mismatch between '%s' and '%s' values for operation '%s'",
			                 fatal_pretty_type_name(left_type), fatal_pretty_type_name(right_type), node->literal_string);

		if(node->op_type == OP_EQUALS)
		{
			if(!is_lvalue(node->first_child))
				VALIDATE_ERROR_LF(node->line_number, node->file, "Can only set to an lvalue");

			free_type(left_type);
			return right_type;
		}

		else if(node->op_type == OP_TEST_EQUAL
		   || node->op_type == OP_TEST_NOT_EQUAL
		   || node->op_type == OP_TEST_GREATER
		   || node->op_type == OP_TEST_GREATER_EQUAL
		   || node->op_type == OP_TEST_LESS
		   || node->op_type == OP_TEST_LESS_EQUAL)
		{
			free_type(left_type);
			free_type(right_type);
			struct TYPE_DATA *type = create_type("Bool");
			type->type = TypeBool;
			return type;
		}

		else if(node->op_type == OP_ADD
				|| node->op_type == OP_SUB
				|| node->op_type == OP_MUL
				|| node->op_type == OP_DIV)
		{
			if(!type_is_number(left_type) || !type_is_number(right_type))
				VALIDATE_ERROR_LF(node->line_number, node->file, "Cannot perform arithmetic on one or more non-numerical values");

			free_type(right_type);
			return left_type;
		}

		else
		{
			printf("INTERNAL ERROR: We don't know how to validate this op");
			exit(EXIT_FAILURE);
		}
	}
	else if(node->node_type == AST_NEGATE)
	{
		assert(count_node_children(node) == 1);

		struct TYPE_DATA *type = typecheck_expression(node->first_child, symbol_table, global, search_using_imports, level + 1);
		if(!type_is_number(type))
			VALIDATE_ERROR_LF(node->line_number, node->file, "Cannot negate non-numerical value");

		return type;
	}

	else if(node->node_type == AST_NEW)
	{
		assert(node->type != NULL);

		if(node->type->child != NULL)
			VALIDATE_ERROR_LF(node->line_number, node->file, "Cannot instance type '%s' as struct '%s' does not support a child type",
			                  fatal_pretty_type_name(node->type), node->type->name);

		struct SYMBOL *symbol = NULL;
		if(node->type->reach_module != NULL)
		{
			struct NODE *reach_module = lookup_module(node->type->reach_module);
			if(reach_module == NULL)
				VALIDATE_ERROR_LF(node->line_number, node->file, "No module has been imported with the name '%s'", node->type->reach_module);
			symbol = lookup_symbol(reach_module->symbol_table, node->type->name, node->file, false);
		}
		else
			symbol = lookup_symbol(symbol_table, node->type->name, node->file, true);

		if(symbol == NULL || symbol->type != SYMBOL_STRUCT)
			VALIDATE_ERROR_LF(node->line_number, node->file, "Cannot instance struct '%s' as it does not exist", node->type->name);

		node->index = symbol->struct_data->node->index;

		int child_count = count_node_children(node);
		int field_count = count_node_children(symbol->struct_data->node->first_child);
		if(child_count != field_count)
			VALIDATE_ERROR_LF(node->line_number, node->file, "Expected %i fields but found %i fields", field_count, child_count);

		struct NODE *child = node->first_child;
		assert(child->node_type == AST_FIELDVALUE);
		struct NODE *field = symbol->struct_data->node->first_child->first_child;
		assert(field->node_type == AST_LET);

		while(child != NULL)
		{
			if(strcmp(field->name, child->name) != 0)
				VALIDATE_ERROR_LF(child->line_number, child->file, "Expected field '%s' but found '%s'", field->name, child->name);

			struct TYPE_DATA *type = typecheck_expression(child->first_child, symbol_table, false, true, 0);
			if(!are_types_equal(field->type, type))
				VALIDATE_ERROR_LF(child->line_number, child->file, "Type mismatch setting field '%s' of type '%s' with expression of type '%s'",
				                  field->name, fatal_pretty_type_name(field->type), fatal_pretty_type_name(type));
			free_type(type);

			child = child->next;
			field = field->next;
		}

		return copy_type(symbol->struct_data->node->type);
	}

	else
	{
		printf("INTERNAL ERROR: Unsupported expression subnode type %i\n", node->node_type);
		exit(EXIT_FAILURE);
	}
}


void prevalidate_block(struct NODE *block, struct SYMBOL_TABLE *symbol_table)
{
	assert(block->symbol_table == NULL);

	block->symbol_table = symbol_table;
	populate_function_symbols(symbol_table, block);

	struct NODE *node = block->first_enum;
	while(node != NULL)
	{
		assert(node->node_type == AST_ENUM);

		struct SYMBOL *symbol = lookup_symbol(symbol_table, node->name, node->file, false);
		if(symbol != NULL)
			VALIDATE_ERROR_LF(node->line_number, node->file, "A symbol named '%s' already exists", node->name);

		symbol = create_symbol(node->name, SYMBOL_ENUM, node->file, node->line_number);
		symbol->enum_data = create_enum(node);
		add_symbol(symbol_table, symbol);

		struct NODE *entry = node->first_child;
		while(entry != NULL)
		{
			entry->index = next_index;
			next_index++;
			entry = entry->next;
		}

		node->index = next_index;
		next_index++;

		free_type(node->type);
		node->type = create_type(node->name);

		node = node->next;
	}

	node = block->first_struct;
	while(node != NULL) //Populate all struct symbols
	{
		assert(node->node_type == AST_STRUCT);

		assert(node->first_child->node_type == AST_BLOCK);
		assert(count_node_children(node) == 1);

		node->index = next_index;
		next_index++;

		free_type(node->type);
		node->type = create_type(node->name);

		struct SYMBOL *symbol = create_symbol(node->name, SYMBOL_STRUCT, node->file, node->line_number);
		symbol->struct_data = create_struct(node);
		add_symbol(symbol_table, symbol);

		node = node->next;
	}

	node = block->first_struct;
	while(node != NULL) //Validate all structs
	{
		assert(node->node_type == AST_STRUCT);

		verify_type_valid(node->type, symbol_table, false, node->line_number, node->file);

		struct NODE *child = node->first_child->first_child;
		while(child != NULL)
		{
			child->index = next_index;
			next_index++;

			verify_type_valid(child->type, symbol_table, false, child->line_number, child->file);

			child = child->next;
		}

		node = node->next;
	}

	if(block->node_type == AST_MODULE)
	{
		node = block->first_child;
		while(node != NULL)
		{
			if(node->node_type == AST_LET) //TODO: Remove this little bit of duplicated code
			{
				node->symbol_table = symbol_table;

				verify_type_valid(node->type, symbol_table, false, node->line_number, node->file);

				struct SYMBOL *symbol = lookup_symbol(symbol_table, node->name, node->file, false);
				if(symbol != NULL)
					VALIDATE_ERROR_LF(node->line_number, node->file, "A symbol named '%s' already exists", node->name);

				node->index = next_index; //Not increasing as add_var will do that when creating the symbol
				struct VAR_DATA *var = create_var(node->type);
				add_var(symbol_table, node->name, var, node->file, node->line_number);
			}

			node = node->next;
		}
	}
}


void validate_block(struct NODE *node, struct SYMBOL_TABLE *symbol_table, bool root, int level) //Exits on failure
{
	assert(node->node_type == AST_BLOCK || node->node_type == AST_MODULE);
	assert(level >= 0);

	struct NODE *block = node;

	if(!root)
		prevalidate_block(node, symbol_table);
	assert(block->symbol_table != NULL);

	if(node->first_import != NULL && !root)
	{
		VALIDATE_ERROR_LF(node->first_import->line_number, node->first_import->file,
		                  "Can only import in the file root scope");
	}

	if(root && node->first_import != NULL)
	{
		struct IMPORT_DATA *import_data = node->first_import;
		while(import_data != NULL)
		{
			struct NODE *module = lookup_module(import_data->name);
			if(module == NULL)
				VALIDATE_ERROR_LF(import_data->line_number, import_data->file,
				                  "No module named '%s'", import_data->name);

			import_data = import_data->next;
		}
	}

	//"foreach" node
	node = node->first_child;
	while(node != NULL)
	{
		//If in root scope disallow execution
		if(root)
		{
			switch(node->node_type)
			{
				case AST_LET:
				case AST_FUNC:
				case AST_TEST:
				case AST_ENUM:
				case AST_STRUCT:
					break;

				default: //TODO: Improve error message
					VALIDATE_ERROR_LF(node->line_number, node->file, "Disallowed in root scope");
					break;
			}
		}

		//Validate the node
		switch(node->node_type)
		{
			case AST_BLOCK:
			{
				validate_block(node, create_symbol_table(symbol_table, node->module), false, level + 1);
				break;
			}

			case AST_EXPRESSION:
			{
				free_type(typecheck_expression(node, symbol_table, root, true, 0));
				break;
			}

			case AST_LET:
			{
				int child_count = count_node_children(node);
				assert(child_count == 0 || child_count == 1);

				if(!root) //If we are in the root of the module then this has already been handled by prevalidate_populate_module
				{
					node->symbol_table = symbol_table;

					verify_type_valid(node->type, symbol_table, false, node->line_number, node->file);

					struct SYMBOL *symbol = lookup_symbol(symbol_table, node->name, node->file, true);
					if(symbol != NULL)
						VALIDATE_ERROR_LF(node->line_number, node->file, "Variable '%s' already exists", node->name);

					node->index = next_index; //Not increasing as add_var will do that when creating the symbol
					struct VAR_DATA *var = create_var(node->type);
					add_var(symbol_table, node->name, var, node->file, node->line_number);
				}

				if(child_count == 1)
				{
					struct TYPE_DATA *expression_type = typecheck_expression(node->first_child, symbol_table, root, true, 0);
					if(!are_types_equal(node->type, expression_type))
					{
						VALIDATE_ERROR_LF(node->line_number, node->file, "Type mismatch declaring variable '%s' of type '%s' with expression of type '%s'",
										 node->name, fatal_pretty_type_name(node->type), fatal_pretty_type_name(expression_type));
					}
					free_type(expression_type);
				}

				break;
			}

			case AST_FUNC:
			case AST_STRUCT:
			case AST_ENUM:
				break;

			case AST_IF:
			case AST_ELIF:
			{
				assert(count_node_children(node) == 2);

				if(node->node_type == AST_ELIF)
				{
					if(node->previous->node_type != AST_IF && node->previous->node_type != AST_ELIF)
						VALIDATE_ERROR_LF(node->line_number, node->file, "An 'elif' statement must follow an 'if' or 'elif'");
				}

				struct TYPE_DATA *expression_type = typecheck_expression(node->first_child, symbol_table, root, true, 0);
				if(strcmp(expression_type->name, "Bool") != 0)
					VALIDATE_ERROR_LF(node->line_number, node->file, "Expected a Bool expression but found expression of type '%s'", expression_type->name);
				free_type(expression_type);

				validate_block(node->last_child, create_symbol_table(symbol_table, node->module), false, level + 1);

				break;
			}

			case AST_ELSE:
			{
				assert(count_node_children(node) == 1);

				if(node->previous->node_type != AST_IF && node->previous->node_type != AST_ELIF)
					VALIDATE_ERROR_LF(node->line_number, node->file, "An 'else' statement must follow an 'if' or 'elif'");

				validate_block(node->last_child, create_symbol_table(symbol_table, node->module), false, level + 1);

				break;
			}

			case AST_WHILE:
			{
				assert(count_node_children(node) == 2);

				struct TYPE_DATA *expression_type = typecheck_expression(node->first_child, symbol_table, root, true, 0);
				if(strcmp(expression_type->name, "Bool") != 0)
					VALIDATE_ERROR_LF(node->line_number, node->file, "Expected a Bool expression but found expression of type '%s'", expression_type->name);
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
					VALIDATE_ERROR_LF(node->line_number, node->file, "Cannot break, no loop found");

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
					VALIDATE_ERROR_LF(node->line_number, node->file, "Cannot continue, no loop found");

				break;
			}

			case AST_RETURN:
			{
				struct NODE *parent = node->parent;
				while(true)
				{
					if(parent->node_type == AST_FUNC || parent->node_type == AST_TEST)
						break;

					parent = parent->parent;
					if(parent == NULL)
						break;
				}
				if(parent == NULL)
					VALIDATE_ERROR_LF(node->line_number, node->file, "Cannot return, not in a function");
				assert(parent->node_type == AST_FUNC || parent->node_type == AST_TEST);

				if(parent->node_type == AST_FUNC)
				{
					if(strcmp(parent->type->name, "Void") != 0 && count_node_children(node) == 0)
					{
						VALIDATE_ERROR_LF(node->line_number, node->file, "Must return a value of type '%s'", parent->type->name);
					}
					else if(count_node_children(node) == 1)
					{
						struct TYPE_DATA *type = typecheck_expression(node->first_child, symbol_table, root, true, level + 1);
						if(!are_types_equal(type, parent->type))
							VALIDATE_ERROR_LF(node->line_number, node->file, "Type mismatch returning a value of type '%s' from a function of type '%s'",
							                  fatal_pretty_type_name(type), fatal_pretty_type_name(parent->type));
						free_type(type);
					}
				}
				else if(parent->node_type == AST_TEST)
				{
					if(count_node_children(node) == 0)
					{
						VALIDATE_ERROR_LF(node->line_number, node->file, "Must return a value of type 'Bool' from test \"%s\"", parent->name);
					}
					else if(count_node_children(node) == 1)
					{
						struct TYPE_DATA *type = typecheck_expression(node->first_child, symbol_table, root, true, level + 1);
						if(strcmp(type->name, "Bool") != 0)
							VALIDATE_ERROR_LF(node->line_number, node->file, "Test \"%s\" return expected type 'Bool' instead of '%s'",
							                  parent->name, fatal_pretty_type_name(type));
						free_type(type);
					}
				}

				break;
			}

			case AST_TEST:
			{
				assert(count_node_children(node) == 1);

				if(!root)
					VALIDATE_ERROR_LF(node->line_number, node->file, "Tests can only exist in module root scope");

				node->index = next_index;
				next_index++;

				validate_block(node->first_child, create_symbol_table(symbol_table, node->module), false, level + 1);
				trace_return(node);

				break;
			}

			default:
				printf("INTERNAL ERROR: We don't know how to validate this statement\n");
				exit(EXIT_FAILURE);
		}

		node = node->next;
	}

	{
		struct SYMBOL_TABLE *root_symbol_table = symbol_table;
		while(root_symbol_table->parent != NULL)
			root_symbol_table = root_symbol_table->parent;

		validate_functions(block, root_symbol_table);
	}

	if(root && strcmp(block->name, "Main") == 0)
	{
		struct SYMBOL *main_func = lookup_symbol(symbol_table, "main", block->file, false);
		if(main_func != NULL)
		{
			if(strcmp(main_func->func_data->return_type->name, "Void") != 0)
				VALIDATE_ERROR_LF(main_func->line, main_func->file, "Function 'main' in module 'Main' must have return type of 'Void'");

			if(main_func->func_data->first_arg != NULL)
				VALIDATE_ERROR_LF(main_func->line, main_func->file, "Function 'main' in module 'Main' should expect no arguments");
		}
	}
}


void populate_function_symbols(struct SYMBOL_TABLE *symbol_table, struct NODE *block)
{
	struct NODE *func_node = block->first_func;
	while(func_node != NULL)
	{
		verify_type_valid(func_node->type, symbol_table, true, func_node->line_number, func_node->file);

		struct ARG_DATA *arg = func_node->first_arg;
		while(arg != NULL)
		{
			verify_type_valid(arg->type, symbol_table, false, func_node->line_number, func_node->file);
			arg = arg->next;
		}

		struct SYMBOL *symbol = create_symbol(func_node->name, SYMBOL_FUNC, func_node->file, func_node->line_number);

		struct FUNC_DATA *func_data = create_func(func_node->type);
		func_data->first_arg = func_node->first_arg;
		func_data->last_arg = func_node->last_arg;
		symbol->func_data = func_data;

		func_node->index = next_index;
		symbol->index = next_index;
		next_index++;

		add_symbol(symbol_table, symbol);

		func_node = func_node->next;
	}
}


bool is_return_satisfied(struct NODE *block)
{
	bool satisfied = false;
	bool if_else_satisfied = false;

	struct NODE *cursor = block->first_child;
	while(cursor != NULL)
	{
		if(cursor->node_type == AST_BLOCK)
		{
			if(is_return_satisfied(cursor))
				return true;
		}

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


void trace_return(struct NODE *func)
{
	assert(func->node_type == AST_FUNC || func->node_type == AST_TEST);

	struct NODE *block = func->first_child;
	assert(block->node_type == AST_BLOCK);

	if(!is_return_satisfied(block))
		VALIDATE_ERROR_LF(func->line_number, func->file, "Not all code paths return a value");
}


void validate_functions(struct NODE *block, struct SYMBOL_TABLE *root_symbol_table)
{
	struct NODE *func_node = block->first_func;
	while(func_node != NULL)
	{
		assert(func_node->node_type == AST_FUNC);
		assert(func_node->first_child->node_type == AST_BLOCK);
		assert(count_node_children(func_node) == 1);

		struct SYMBOL_TABLE *symbol_table = create_symbol_table(root_symbol_table, func_node->module);
		func_node->symbol_table = symbol_table;

		verify_type_valid(func_node->type, symbol_table, true, func_node->line_number, func_node->file);

		struct ARG_DATA *arg = func_node->first_arg;
		while(arg != NULL)
		{
			verify_type_valid(arg->type, symbol_table, false, func_node->line_number, func_node->file);

			arg->index = next_index; //next_index is incremented in add_var
			add_var(symbol_table, arg->name, create_var(arg->type), arg->file, arg->line_number);

			arg = arg->next;
		}

		validate_block(func_node->first_child, symbol_table, false, 0);
		if(strcmp(func_node->type->name, "Void") != 0)
			trace_return(func_node);


		func_node = func_node->next;
	}
}

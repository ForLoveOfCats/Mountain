#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ast.h"
#include "validator.h"
#include "transpile.h"



void prepare_file(FILE *target)
{
	fprintf(target, "#include <stdint.h>\n#include <stdbool.h>\n\n\n\n");
}


char *type_to_c(char *type)
{
	if(strcmp(type, "i32") == 0)
		return "int32_t";

	if(strcmp(type, "Bool") == 0)
		return "bool";

	printf("Invalid type to transpile '%s'\n", type);
	exit(EXIT_FAILURE);
}


void transpile_expression(FILE *target, struct NODE *node) //TODO support non-numerical values
{
	if(node->type == AST_EXPRESSION)
	{
		transpile_expression(target, node->first_child);
		return;
	}

	fprintf(target, "(");

	if(node->type == AST_OP)
	{
		transpile_expression(target, node->first_child);
		switch(node->op_type)
		{
			case OP_TEST_EQUAL:
				fprintf(target, " == ");
				break;

			case OP_TEST_NOT_EQUAL:
				fprintf(target, " != ");
				break;

			case OP_TEST_GREATER:
				fprintf(target, " > ");
				break;

			case OP_TEST_GREATER_EQUAL:
				fprintf(target, " >= ");
				break;

			case OP_TEST_LESS:
				fprintf(target, " < ");
				break;

			case OP_TEST_LESS_EQUAL:
				fprintf(target, " <= ");
				break;

			case OP_ADD:
				fprintf(target, " + ");
				break;

			case OP_SUB:
				fprintf(target, " - ");
				break;

			case OP_MUL:
				fprintf(target, " * ");
				break;

			case OP_DIV:
				fprintf(target, " / ");
				break;

			default:
				printf("INTERNAL ERROR: We don't know how to transpile this op\n");
				exit(EXIT_FAILURE);
		}
		transpile_expression(target, node->first_child->next);
	}
	else if(node->type == AST_UNOP)
	{
		fprintf(target, "!");
		transpile_expression(target, node->first_child);
	}
	else if(node->type == AST_GET)
	{
		fprintf(target, "var_%i", node->index);
	}
	else
		fprintf(target, "%s", node->literal_string);

	fprintf(target, ")");
}


void transpile_var(FILE *target, struct NODE *node)
{
	assert(node->index >= 0);
	fprintf(target, "%s var_%i = ", type_to_c(node->type_name), node->index);
	transpile_expression(target, node->first_child);
	fprintf(target, ";\n");
}


void prototype_globals(FILE *target, struct NODE *root)
{
	assert(root->type == AST_BLOCK);
	struct NODE *node = root->first_child;
	while(node != NULL)
	{
		if(node->type == AST_VAR)
		{
			fprintf(target, "%s var_%i;\n", type_to_c(node->type_name), node->index);
		}

		node = node->next;
	}

	fprintf(target, "\n\n\n");
}


void transpile_function_signature(FILE *target, struct NODE *node)
{
	fprintf(target, "%s func_%i(", type_to_c(node->type_name), node->index);
	struct ARG_DATA *arg = node->first_arg;
	while(arg != NULL)
	{
		fprintf(target, "%s arg_%i", type_to_c(arg->type), arg->index);
		if(arg->next != NULL)
			fprintf(target, ",");

		arg = arg->next;
	}
	fprintf(target, ")");
}


void prototype_functions(FILE *target)
{
	struct NODE *node = first_function;
	while(node != NULL)
	{
		transpile_function_signature(target, node);
		fprintf(target, ";\n");

		node = node->next;
	}

	fprintf(target, "\n\n\n");
}


void transpile_functions(FILE *target)
{
	struct NODE *node = first_function;
	while(node != NULL)
	{
		transpile_function_signature(target, node);
		transpile_block(target, node->first_child, 0);

		node = node->next;
	}

	fprintf(target, "\n\n\n");
}


void transpile_block(FILE *target, struct NODE *node, int level) //This is in no way efficient...
{
	assert(level >= 0);
	assert(node->type == AST_BLOCK);
	node = node->first_child;

	fprintf(target, "{\n");

	//"foreach" node
	while(node != NULL)
	{
		switch(node->type)
		{
			case AST_BLOCK:
				transpile_block(target, node, level + 1);
				break;

			case AST_VAR:
				if(level > 0)
					transpile_var(target, node);
				else
				{
					assert(node->index >= 0);
					fprintf(target, "var_%i = ", node->index);
					transpile_expression(target, node->first_child);
					fprintf(target, ";\n");
				}
				break;

			case AST_SET:
				assert(node->index >= 0);
				fprintf(target, "var_%i = ", node->index);
				transpile_expression(target, node->first_child);
				fprintf(target, ";\n");
				break;

			case AST_IF:
				assert(count_node_children(node) == 2);
				fprintf(target, "if");
				transpile_expression(target, node->first_child);
				fprintf(target, "\n");
				transpile_block(target, node->last_child, level + 1);
				break;

			case AST_WHILE:
				assert(count_node_children(node) == 2);
				fprintf(target, "while");
				transpile_expression(target, node->first_child);
				fprintf(target, "\n");
				transpile_block(target, node->last_child, level + 1);
				break;

			case AST_FUNC:
				break; //This is transpiled elsewhere

			default:
				printf("INTERNAL ERROR: We don't know how to transpile this statement\n");
				exit(EXIT_FAILURE);
		}

		node = node->next;
	}

	fprintf(target, "}\n");
}

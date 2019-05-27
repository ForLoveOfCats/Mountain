#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ast.h"
#include "validator.h"


char *type_to_c(char *type)
{
	if(strcmp(type, "Int32") == 0)
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


void transpile_block(FILE *target, struct NODE *node, int level) //This is in no way efficient...
{
	assert(node->type == AST_BLOCK);
	node = node->first_child;

	assert(level >= 0);
	bool inner_block = level > 0;
	if(!inner_block)
	{
		fprintf(target, "#include <stdint.h>\n#include <stdbool.h>\n\n");
		fprintf(target, "int main()\n");
	}

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
				assert(node->index >= 0);
				fprintf(target, "%s var_%i = ", type_to_c(node->type_name), node->index);
				transpile_expression(target, node->first_child);
				fprintf(target, ";\n");
				break;

			case AST_SET:
				assert(node->index >= 0);
				fprintf(target, "var_%i = ", node->index);
				transpile_expression(target, node->first_child);
				fprintf(target, ";\n");
				break;

			default:
				break;
		}

		node = node->next;
	}

	fprintf(target, "}\n");
}

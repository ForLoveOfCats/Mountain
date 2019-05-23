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
	{
	  return "int32_t";
	}

	printf("Invalid type to transpile '%s'\n", type);
	exit(EXIT_FAILURE);
}


void transpile_expression(FILE *target, struct NODE *node) //TODO support non-numerical values
{
	if(node->type == AST_EXPRESSION)
	{
		printf("Expression node\n");
		transpile_expression(target, node->first_child);
		return;
	}
	printf("Children %i\n", count_node_children(node));

	fprintf(target, "(");

	if(node->type == AST_OP)
	{
		transpile_expression(target, node->first_child);
		switch(node->op_type)
		{
			case OP_ADD:
				fprintf(target, " + ");
				break;

			case OP_SUB:
				fprintf(target, " - ");
				break;

			default:
				printf("INTERNAL ERROR: We don't know how to transpile this op\n");
				exit(EXIT_FAILURE);
		}
		transpile_expression(target, node->first_child->next);
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
		fprintf(target, "#include <stdint.h>\n\n");
		fprintf(target, "int main() {\n");
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

	if(inner_block)
		fprintf(target, "}\n");
	else
		fprintf(target, "}\n}\n");
}

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ast.h"


char *type_to_c(char *type)
{
	if(strcmp(type, "Int32") == 0)
	{
	  return "int32_t";
	}

	printf("Invalid type to transpile '%s'\n", type);
	exit(EXIT_FAILURE);
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
				fprintf(target, "%s var_%i = %s;\n", type_to_c(node->type_name), node->index, node->first_child->literal_string);
				break;

			case AST_SET:
				assert(node->index >= 0);
				fprintf(target, "var_%i = %s;\n", node->index, node->first_child->literal_string);
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

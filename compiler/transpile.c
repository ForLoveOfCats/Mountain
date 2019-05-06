#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ast.h"


void transpile_block(FILE *target, struct NODE *node, int level) //This is in no way efficient...
{
	assert(node->type == AST_BLOCK);
	node = node->first_child;

	assert(level >= 0);
	bool inner_block = level > 0;
	if(!inner_block)
	{
		fprintf(target, "#include <stdint.h>\n\n");
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

			case AST_VAR: //TODO Support non-int types
				assert(strcmp(node->type_name, "Int") == 0);
				fprintf(target, "int32_t var_%i = %s;\n", node->index, node->first_child->literal_string);
				break;

			case AST_SET:
				assert(strcmp(node->type_name, "Int") == 0);
				fprintf(target, "var_%i = %s;\n", node->index, node->first_child->literal_string);
				break;

			default:
				break;
		}

		node = node->next;
	}

	fprintf(target, "}\n");
}

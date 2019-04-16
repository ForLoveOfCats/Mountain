#include <stdlib.h>
#include <stdio.h>

#include "compiler.h"
#include "ast.h"


struct NODE create_node(enum AST_TYPE type)
{
	struct NODE new_node = {type, NULL, NULL, NULL, 0, 0};
	return new_node;
}


void add_node(struct NODE new_node)
{
	printf("Added node with stack_location of %i\n", new_node.stack_location);
}


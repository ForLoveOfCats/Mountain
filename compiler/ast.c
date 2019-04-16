#include <stdlib.h>

#include "ast.h"


struct NODE create_node(enum AST_TYPE type)
{
	struct NODE new_node = {type, NULL, NULL, NULL, 0};
	return new_node;
}


void add_node(struct NODE new_node)
{}


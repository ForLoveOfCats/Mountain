#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ast.h"
#include "validator.h"


void validate_tree(struct NODE *node) //Exits on failure
{
	assert(node->type == AST_ROOT);
	node = node->first_child;

	//"foreach" node
	while(node != NULL)
	{
		switch(node->type)
		{
			case AST_DEF:
			{
				assert(count_node_children(node) == 1);

				//TODO Check that the variable doesn't already exist

				if(strcmp(node->type_name, "Int") != 0) //TODO allow non-int values
				{
					printf("Validation Error: Variable defined with non-Int type on line %i\n", node->line_number);
					exit(EXIT_FAILURE);
				}

				break;
			}

			case AST_MUT:
			{
				assert(count_node_children(node) == 1);

				//TODO Check that the variable doesn't already exist

				if(strcmp(node->type_name, "Int") != 0) //TODO allow non-int values
				{
					printf("Validation Error: Variable defined with non-Int type on line %i\n", node->line_number);
					exit(EXIT_FAILURE);
				}

				break;
			}

			case AST_SET:
			{
				assert(count_node_children(node) == 1);
				//TODO Check that variable exists and is mutable
			}

			default: //To get ccls to shut up
				break;
		}

		node = node->next_node;
	}
}

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "ast.h"
#include "parser.h"


int main(int arg_count, char *arg_array[])
{
	if(arg_count != 2)
	{
		printf("Please provide a file path to compile\n");
		return EXIT_FAILURE;
	}

	FILE *source_file = fopen(arg_array[1], "r");
	if(source_file == NULL)
	{
		printf("Could not open file '%s' to compile\n", arg_array[1]);
		return EXIT_FAILURE;
	}

	root_node = create_node(AST_ROOT);
	current_parse_parent_node = root_node;

	while(parse_next_statement(source_file)) {}

	free_node(root_node);

	fclose(source_file);
	return EXIT_SUCCESS;
}

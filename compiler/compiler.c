#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "compiler.h"
#include "ast.h"
#include "parser.h"
#include "validator.h"
#include "transpile.h"


FILE *open_source_file(char *path)
{
	FILE *source_file = fopen(path, "r");
	if(source_file == NULL)
	{
		printf("Could not open file '%s' to compile\n", path);
		exit(EXIT_FAILURE);
	}

	current_file_character = 0;
	current_file_line = 1;

	return source_file;
}


int main(int arg_count, char *arg_array[])
{
	if(arg_count != 2)
	{
		printf("Please provide a file path to compile\n");
		return EXIT_FAILURE;
	}

	FILE *source_file = open_source_file(arg_array[1]);

	root_node = create_node(AST_BLOCK, -1);
	current_parse_parent_node = root_node;

	struct TOKEN *first_token = tokenize_file(source_file);
	parse_block(first_token, 0);
	free_token_list(first_token);

	struct SCOPE *scope = create_scope(NULL);
	validate_block(root_node, scope, 0);
	free_scope_tree(scope);

	free_tree(root_node);

	fclose(source_file);
	return EXIT_SUCCESS;
}

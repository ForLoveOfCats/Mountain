#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "compiler.h"
#include "ast.h"
#include "parser.h"
#include "symbols.h"
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
	if(arg_count != 3)
	{
		printf("Please provide an output path and an input path to compile\n");
		return EXIT_FAILURE;
	}

	FILE *source_file = open_source_file(arg_array[2]);

	root_node = create_node(AST_BLOCK, -1, -1, -1);
	current_parse_parent_node = root_node;

	struct TOKEN *first_token = tokenize_file(source_file);
	parse_block(first_token, false, 0);
	free_token_list(first_token);

	next_index = 0;
	struct SYMBOL_TABLE *table = create_symbol_table(NULL);
	validate_block(root_node, table, 0);
	validate_functions(table); //TODO: Validate functions first, requires changes to disallow execution in global scope

	FILE *output_file = fopen(arg_array[1], "w");
	prepare_file(output_file);
	prototype_globals(output_file, root_node);
	prototype_functions(output_file);
	transpile_functions(output_file);
	fprintf(output_file, "int main()\n");;
	transpile_block(output_file, root_node, 0);
	fclose(output_file);

	free_table_tree(table);
	free_tree(root_node);

	fclose(source_file);
	return EXIT_SUCCESS;
}

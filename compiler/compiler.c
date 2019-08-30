#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>

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
		printf("Please provide an output file to emit to and an input dir to compile\n");
		return EXIT_FAILURE;
	}


	#define input_path arg_array[2]
	DIR *input_dir = opendir(input_path);
	if(input_dir == NULL)
	{
		printf("Please provide a valid input directory\n");
		exit(EXIT_FAILURE);
	}

	int file_count = 0;
	FILE **files = malloc(sizeof(FILE*) * 0);
	char **paths = malloc(sizeof(char) * 0);
	struct dirent *ent;
	while((ent = readdir(input_dir)) != NULL)
	{
		switch(ent->d_type)
		{
			case DT_REG:
			case DT_LNK:
			{

				char *path = malloc(sizeof(char) * (strlen(input_path) + strlen(ent->d_name) + 2));
				sprintf(path, "%s/%s", input_path, ent->d_name);
				char *old_path = path;
				path = realpath(path, NULL);
				free(old_path);

				char **new_paths = malloc(sizeof(char*) * (file_count+1));
				for(int index = 0; index < file_count; index++)
				{
					new_paths[index] = paths[index];
				}
				new_paths[file_count] = path;

				free(paths);
				paths = new_paths;

				FILE *source_file = open_source_file(path);

				FILE **new_files = malloc(sizeof(FILE*) * (file_count+1));
				for(int index = 0; index < file_count; index++)
				{
					new_files[index] = files[index];
				}
				new_files[file_count] = source_file;

				free(files);
				files = new_files;

				file_count++;
				break;
			}

			default:
				break;
		}
	}
	closedir(input_dir);

	first_func_prototype = NULL;
	last_func_prototype = NULL;
	root_node = create_node(AST_BLOCK, -1, -1, -1, -1);
	current_parse_parent_node = root_node;

	for(int index = 0; index < file_count; index++)
	{
		current_file = index;
		parse_file(files[index]);
	}
	current_file = -1;

	next_index = 0;
	root_symbol_table = create_symbol_table(NULL);
	validate_block(root_node, root_symbol_table, true, 0);

	FILE *output_file = fopen(arg_array[1], "w");
	prepare_file(output_file);
	prototype_globals(output_file, root_node);
	prototype_functions(output_file);
	transpile_functions(output_file);

	fprintf(output_file, "int main()\n{\n");;
	transpile_global_sets(output_file, root_node);
	fprintf(output_file, "\n\nsymbol_%i();\n}\n", lookup_symbol(root_symbol_table, "main")->index);

	fclose(output_file);

	free_table_tree(root_symbol_table);
	free_tree(root_node);
	free_func_prototype_list(first_func_prototype);

	for(int index = 0; index < file_count; index++)
	{
		fclose(files[index]);
		free(paths[index]);
	}
	free(files);

	return EXIT_SUCCESS;
}

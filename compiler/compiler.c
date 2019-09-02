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
	FILE **files = NULL;
	paths = NULL;
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

	first_module = NULL;
	last_module = NULL;
	first_func_prototype = NULL;
	last_func_prototype = NULL;

	for(int index = 0; index < file_count; index++)
	{
		current_file = index;

		current_file_character = 0;
		current_file_line = 1;

		parse_file(files[index]);
	}
	current_file = -1;

	next_index = 0;
	struct NODE *current_module = first_module;
	while(current_module != NULL)
	{
		struct SYMBOL_TABLE *root_symbol_table = create_symbol_table(NULL);
		prevalidate_populate_module(current_module, root_symbol_table);

		current_module = current_module->next;
	}
	current_module = first_module;
	while(current_module != NULL)
	{
		validate_block(current_module, current_module->symbol_table, true, 0);
		current_module = current_module->next;
	}

	FILE *output_file = fopen(arg_array[1], "w");
	prepare_file(output_file);
	{
		current_module = first_module;
		while(current_module != NULL)
		{
			prototype_globals(output_file, current_module);
			current_module = current_module->next;
		}
	}

	prototype_functions(output_file);
	transpile_functions(output_file);

	fprintf(output_file, "int main()\n{\n");;
	{
		current_module = first_module;
		while(current_module != NULL)
		{
			transpile_global_sets(output_file, current_module);
			current_module = current_module->next;
		}
	}

	{
		current_module = first_module;
		while(current_module != NULL)
		{
			if(strcmp(current_module->name, "Main") == 0)
				fprintf(output_file, "\n\nsymbol_%i();\n}\n", lookup_symbol(current_module->symbol_table, "main")->index);

			current_module = current_module->next;
		}
	}

	fclose(output_file);

	{
		current_module = first_module;
		while(current_module != NULL)
		{
			free_table_tree(current_module->symbol_table);

			struct NODE *next_module = current_module->next;
			free_tree(current_module);
			current_module = next_module;
		}
	}

	free_func_prototype_list(first_func_prototype);

	for(int index = 0; index < file_count; index++)
	{
		fclose(files[index]);
		free(paths[index]);
	}
	free(files);

	return EXIT_SUCCESS;
}

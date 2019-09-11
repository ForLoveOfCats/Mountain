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



enum COMPILE_ACTION {ACTION_BUILD, ACTION_TEST};

enum COMPILE_ACTION compile_action;


char null_termination = '\0';
char *get_extension(char *path) //Performs no allocations, do not free!
{
	char *dot = strrchr(path, '.');
	if(dot == NULL)
		return &null_termination; //To not error a strcmp

	return dot+1; //Move to the next character
}


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
	if(arg_count != 4)
	{
		printf("Please provide an valid action, an input dir to compile, and an output file to emit to\n");
		return EXIT_FAILURE;
	}

	#define action_string arg_array[1]
	#define input_path arg_array[2]
	#define output_path arg_array[3]

	if(strcmp(action_string, "build") == 0)
		compile_action = ACTION_BUILD;
	else if(strcmp(action_string, "test") == 0)
		compile_action = ACTION_TEST;
	else
	{
		printf("CLI Error: Invalid action '%s', valid actions are 'build' and 'test'\n", action_string);
		exit(EXIT_FAILURE);
	}


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
				if(strcmp(get_extension(ent->d_name), "mtn") != 0)
					continue;

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
		struct SYMBOL_TABLE *root_symbol_table = create_symbol_table(NULL, current_module);
		prevalidate_populate_module(current_module, root_symbol_table);

		current_module = current_module->next;
	}
	current_module = first_module;
	while(current_module != NULL)
	{
		validate_block(current_module, current_module->symbol_table, true, 0);
		current_module = current_module->next;
	}

	FILE *output_file = fopen(output_path, "w");
	prepare_file(output_file);
	{
		current_module = first_module;
		while(current_module != NULL)
		{
			transpile_global_enums(output_file, current_module);
			current_module = current_module->next;
		}

		current_module = first_module;
		while(current_module != NULL)
		{
			prototype_globals(output_file, current_module);
			current_module = current_module->next;
		}
	}

	prototype_functions(output_file);
	transpile_functions(output_file);
	if(compile_action == ACTION_TEST)
	{
		struct NODE *module = first_module;
		while(module != NULL)
		{
			transpile_tests(output_file, module);
			module = module->next;
		}
		fprintf(output_file, "\n\n\n\n");
	}

	fprintf(output_file, "int main()\n{\n");;
	fprintf(output_file, "assert(sizeof(int) == sizeof(int32_t));\n");;
	fprintf(output_file, "assert(sizeof(char) == sizeof(uint8_t));\n\n");;
	{
		current_module = first_module;
		while(current_module != NULL)
		{
			transpile_global_sets(output_file, current_module);
			current_module = current_module->next;
		}
	}

	if(compile_action == ACTION_BUILD)
	{
		current_module = first_module;
		while(current_module != NULL)
		{
			if(strcmp(current_module->name, "Main") == 0)
			{
				struct SYMBOL *main_symbol = lookup_symbol(current_module->symbol_table, "main", -1, false);
				if(main_symbol != NULL)
					fprintf(output_file, "\n\nsymbol_%i();\n", main_symbol->index);

				break;
			}

			current_module = current_module->next;
		}
	}
	else if(compile_action == ACTION_TEST)
		transpile_test_calls(output_file);

	fprintf(output_file, "\n\nexit(EXIT_SUCCESS);\n}\n");
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

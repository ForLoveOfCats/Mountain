#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ast.h"
#include "validator.h"
#include "transpile.h"



void prepare_file(FILE *target)
{
	fprintf(target, "#include <stdint.h>\n#include <stdbool.h>\n\n\n\n");
}


char *type_to_c(char *type) //TODO: Make this not suck
{
	if(strcmp(type, "i32") == 0)
		return "int32_t";

	if(strcmp(type, "Bool") == 0)
		return "bool";

	if(strcmp(type, "Void") == 0)
		return "void";

	printf("Invalid type to transpile '%s'\n", type);
	exit(EXIT_FAILURE);
}


void transpile_expression(FILE *target, struct NODE *node) //TODO support non-numerical values
{
	if(node->node_type == AST_EXPRESSION)
	{
		transpile_expression(target, node->first_child);
		return;
	}

	fprintf(target, "(");

	if(node->node_type == AST_OP)
	{
		transpile_expression(target, node->first_child);
		switch(node->op_type)
		{
			case OP_TEST_EQUAL:
				fprintf(target, " == ");
				break;

			case OP_TEST_NOT_EQUAL:
				fprintf(target, " != ");
				break;

			case OP_TEST_GREATER:
				fprintf(target, " > ");
				break;

			case OP_TEST_GREATER_EQUAL:
				fprintf(target, " >= ");
				break;

			case OP_TEST_LESS:
				fprintf(target, " < ");
				break;

			case OP_TEST_LESS_EQUAL:
				fprintf(target, " <= ");
				break;

			case OP_ADD:
				fprintf(target, " + ");
				break;

			case OP_SUB:
				fprintf(target, " - ");
				break;

			case OP_MUL:
				fprintf(target, " * ");
				break;

			case OP_DIV:
				fprintf(target, " / ");
				break;

			default:
				printf("INTERNAL ERROR: We don't know how to transpile this op\n");
				exit(EXIT_FAILURE);
		}
		transpile_expression(target, node->first_child->next);
	}
	else if(node->node_type == AST_UNOP)
	{
		fprintf(target, "!");
		transpile_expression(target, node->first_child);
	}
	else if(node->node_type == AST_GET)
	{
		fprintf(target, "symbol_%i", node->index);
	}
	else if(node->node_type == AST_CALL)
	{
		fprintf(target, "symbol_%i(", node->index);

		struct NODE *arg = node->first_child;
		while(arg != NULL)
		{
			transpile_expression(target, arg);
			if(arg->next != NULL)
				fprintf(target, ", ");

			arg = arg->next;
		}

		fprintf(target, ")");
	}
	else
		fprintf(target, "%s", node->literal_string);

	fprintf(target, ")");
}


void prototype_globals(FILE *target, struct NODE *root)
{
	assert(root->node_type == AST_BLOCK);
	struct NODE *node = root->first_child;
	while(node != NULL)
	{
		if(node->node_type == AST_VAR)
		{
			fprintf(target, "%s symbol_%i;\n", type_to_c(node->type->name), node->index);
		}

		node = node->next;
	}

	fprintf(target, "\n\n\n");
}


void transpile_function_signature(FILE *target, struct NODE *node)
{
	fprintf(target, "%s symbol_%i(", type_to_c(node->type->name), node->index);
	struct ARG_DATA *arg = node->first_arg;
	while(arg != NULL)
	{
		fprintf(target, "%s symbol_%i", type_to_c(arg->type->name), arg->index);
		if(arg->next != NULL)
			fprintf(target, ", ");

		arg = arg->next;
	}
	fprintf(target, ")");
}


void prototype_functions(FILE *target)
{
	struct FUNC_PROTOTYPE *prototype = first_func_prototype;
	while(prototype != NULL)
	{
		transpile_function_signature(target, prototype->func);
		fprintf(target, ";\n");

		prototype = prototype->next;
	}

	fprintf(target, "\n\n\n");
}


void transpile_functions(FILE *target)
{
	struct FUNC_PROTOTYPE *prototype = first_func_prototype;
	while(prototype != NULL)
	{
		transpile_function_signature(target, prototype->func);
		transpile_block(target, prototype->func->first_child, 0);
		fprintf(target, "\n");

		prototype = prototype->next;
	}

	fprintf(target, "\n\n\n");
}


void transpile_block(FILE *target, struct NODE *node, int level) //This is in no way efficient...
{
	assert(level >= 0);
	assert(node->node_type == AST_BLOCK);
	node = node->first_child;

	fprintf(target, "{\n");

	//"foreach" node
	while(node != NULL)
	{
		switch(node->node_type)
		{
			case AST_BLOCK:
				transpile_block(target, node, level + 1);
				break;

			case AST_VAR:
				assert(node->index >= 0);
				fprintf(target, "%s symbol_%i", type_to_c(node->type->name), node->index);
				if(count_node_children(node) == 1)
				{
					fprintf(target, " = ");
					transpile_expression(target, node->first_child);
				}
				fprintf(target, ";\n");
				break;

			case AST_SET:
				assert(node->index >= 0);
				fprintf(target, "symbol_%i = ", node->index);
				transpile_expression(target, node->first_child);
				fprintf(target, ";\n");
				break;

			case AST_IF:
				assert(count_node_children(node) == 2);
				fprintf(target, "if");
				transpile_expression(target, node->first_child);
				fprintf(target, "\n");
				transpile_block(target, node->last_child, level + 1);
				break;

			case AST_ELIF:
				assert(count_node_children(node) == 2);
				fprintf(target, "else if");
				transpile_expression(target, node->first_child);
				fprintf(target, "\n");
				transpile_block(target, node->last_child, level + 1);
				break;

			case AST_ELSE:
				assert(count_node_children(node) == 1);
				fprintf(target, "else\n");
				transpile_block(target, node->last_child, level + 1);
				break;

			case AST_WHILE:
				assert(count_node_children(node) == 2);
				fprintf(target, "while");
				transpile_expression(target, node->first_child);
				fprintf(target, "\n");
				transpile_block(target, node->last_child, level + 1);
				break;

			case AST_BREAK:
				assert(count_node_children(node) == 0);
				fprintf(target, "break;\n");
				break;

			case AST_CONTINUE:
				assert(count_node_children(node) == 0);
				fprintf(target, "continue;\n");
				break;

			case AST_RETURN:
				if(count_node_children(node) == 0)
					fprintf(target, "return;\n");
				else
				{
					assert(count_node_children(node) == 1);

					fprintf(target, "return ");
					transpile_expression(target, node->first_child);
					fprintf(target, ";\n");
				}

				break;

			case AST_CALL:
				fprintf(target, "symbol_%i(", node->index);

				struct NODE *arg = node->first_child;
				while(arg != NULL)
				{
					transpile_expression(target, arg);
					if(arg->next != NULL)
						fprintf(target, ", ");

					arg = arg->next;
				}

				fprintf(target, ");\n");
				break;

			case AST_FUNC:
				break; //This is transpiled elsewhere

			case AST_STRUCT:
				break; //This is transpiled elsewhere

			default:
				printf("INTERNAL ERROR: We don't know how to transpile this statement\n");
				exit(EXIT_FAILURE);
		}

		node = node->next;
	}

	fprintf(target, "}\n");
}

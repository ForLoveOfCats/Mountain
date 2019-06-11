#pragma once
#include <stdint.h>


#include "symbols.h"



struct NODE *root_node;
struct NODE *current_parse_parent_node;

struct NODE *first_function;
struct NODE *last_function;


enum AST_TYPE {AST_BLOCK, AST_IF, AST_WHILE, AST_FUNC, AST_VAR, AST_SET, AST_GET, AST_EXPRESSION, AST_LITERAL, AST_OP, AST_UNOP};
enum OP_TYPE {OP_NONE, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_TEST_EQUAL, OP_TEST_NOT_EQUAL, OP_TEST_GREATER, OP_TEST_GREATER_EQUAL, OP_TEST_LESS, OP_TEST_LESS_EQUAL};
enum UNOP_TYPE {UNOP_NONE, UNOP_INVERT};
enum LITERAL_TYPE {LITERAL_NUMBER, LITERAL_BOOL};


struct NODE
{
	enum AST_TYPE type;

	struct NODE *parent;
	struct NODE *next;
	struct NODE *previous;
	struct NODE *first_child;
	struct NODE *last_child;

	struct ARG_DATA *first_arg; //For func define and call
	struct ARG_DATA *last_arg;

	int line_number;
	int start_char;
	int end_char;

	int index;

	struct TYPE_DATA *type_name; //For var, get, and set
	char *variable_name; //var's and set's name
	char *function_name; //for function declaration and calling

	enum LITERAL_TYPE literal_type;
	char *literal_string;
	bool literal_bool;

	enum OP_TYPE op_type;
	enum UNOP_TYPE unop_type;
};


struct ARG_DATA
{
	struct ARG_DATA *next;

	char *name;
	char *type;

	int index;
};


struct NODE *create_node(enum AST_TYPE type, int line_number, int start_char, int end_char);


void add_node(struct NODE *parent, struct NODE *new_node);


void remove_node(struct NODE *parent, struct NODE *child);


int count_node_children(struct NODE *node);


int recursive_count_node_children(struct NODE *node);


void free_tree(struct NODE *child);


struct ARG_DATA *create_arg_data(char *name, char *type);

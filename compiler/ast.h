#pragma once
#include <stdint.h>



struct NODE *root_node;
struct NODE *current_parse_parent_node;



enum AST_TYPE {AST_BLOCK, AST_DEF, AST_MUT, AST_SET, AST_LITERAL};
enum LITERAL_TYPE {LITERAL_INT};

struct NODE
{
	enum AST_TYPE type;

	struct NODE *parent;
	struct NODE *next;
	struct NODE *first_child;
	struct NODE *last_child;

	int line_number;

	char *type_name; //For def and get

	uint32_t stack_len;
	uint32_t def_location; //Where in the stack the var is if def or get

	char *variable_name; //def, var, get, and set name

	enum LITERAL_TYPE literal_type;
	char *literal_string;
};


struct NODE *create_node(enum AST_TYPE type, int line_number);


void add_node(struct NODE *parent, struct NODE *new_node);


int count_node_children(struct NODE *node);


void free_tree(struct NODE *child);

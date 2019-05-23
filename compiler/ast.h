#pragma once
#include <stdint.h>



struct NODE *root_node;
struct NODE *current_parse_parent_node;



enum AST_TYPE {AST_BLOCK, AST_VAR, AST_SET, AST_EXPRESSION, AST_LITERAL, AST_OP};
enum OP_TYPE {OP_NONE, OP_ADD, OP_SUB};
enum LITERAL_TYPE {LITERAL_INT};

struct NODE
{
	enum AST_TYPE type;

	struct NODE *parent;
	struct NODE *next;
	struct NODE *first_child;
	struct NODE *last_child;

	int line_number;

	int index;

	char *type_name; //For var, get, and set
	char *variable_name; //var's and set's name

	enum LITERAL_TYPE literal_type;
	char *literal_string;

	enum OP_TYPE op_type;
};


struct NODE *create_node(enum AST_TYPE type, int line_number);


void add_node(struct NODE *parent, struct NODE *new_node);


int count_node_children(struct NODE *node);


int recursive_count_node_children(struct NODE *node);


void free_tree(struct NODE *child);

#pragma once
#include <stdint.h>



struct NODE *root_node;
struct NODE *current_parse_parent_node;



enum AST_TYPE {AST_BLOCK, AST_VAR, AST_SET, AST_GET, AST_EXPRESSION, AST_LITERAL, AST_OP, AST_UNOP};
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

	int line_number;
	int start_char;
	int end_char;

	int index;

	char *type_name; //For var, get, and set
	char *variable_name; //var's and set's name

	enum LITERAL_TYPE literal_type;
	char *literal_string;
	bool literal_bool;

	enum OP_TYPE op_type;
	enum UNOP_TYPE unop_type;
};


struct NODE *create_node(enum AST_TYPE type, int line_number, int start_char, int end_char);


void add_node(struct NODE *parent, struct NODE *new_node);


void remove_node(struct NODE *parent, struct NODE *child);


int count_node_children(struct NODE *node);


int recursive_count_node_children(struct NODE *node);


void free_tree(struct NODE *child);

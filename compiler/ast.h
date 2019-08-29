#pragma once
#include <stdint.h>


#include "symbols.h"



struct NODE *root_node;
struct NODE *current_parse_parent_node;


struct FUNC_PROTOTYPE *first_func_prototype;
struct FUNC_PROTOTYPE *last_func_prototype;


enum AST_TYPE {AST_BLOCK, AST_IF, AST_ELIF, AST_ELSE, AST_WHILE, AST_BREAK, AST_CONTINUE, AST_FUNC, AST_STRUCT, AST_LET, AST_GET, AST_CALL, AST_RETURN, AST_EXPRESSION, AST_LITERAL, AST_OP, AST_UNOP};
enum OP_TYPE {OP_NONE, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_EQUALS, OP_TEST_EQUAL, OP_TEST_NOT_EQUAL, OP_TEST_GREATER, OP_TEST_GREATER_EQUAL, OP_TEST_LESS, OP_TEST_LESS_EQUAL};
enum UNOP_TYPE {UNOP_NONE, UNOP_INVERT, UNOP_ADDRESS_OF, UNOP_DEREFERENCE};
enum LITERAL_TYPE {LITERAL_NUMBER, LITERAL_BOOL};


struct NODE
{
	enum AST_TYPE node_type;

	struct NODE *parent;
	struct NODE *next;
	struct NODE *previous;
	struct NODE *first_child;
	struct NODE *last_child;

	struct NODE *first_func;
	struct NODE *last_func;

	struct ARG_DATA *first_arg; //For function definitions (not calls)
	struct ARG_DATA *last_arg;

	int line_number;
	int start_char;
	int end_char;

	struct SYMBOL_TABLE *symbol_table; //Not freed with NODE, freed in seperate call to free_table_tree
	int index;

	struct TYPE_DATA *type; //For var, get, and set
	char *name; //var's and set's name

	enum LITERAL_TYPE literal_type;
	char *literal_string;
	bool literal_bool;

	enum OP_TYPE op_type;
	enum UNOP_TYPE unop_type;
};


struct FUNC_PROTOTYPE
{
	struct NODE *func; //The actual AST_FUNC node
	struct FUNC_PROTOTYPE *next;
};


struct NODE *create_node(enum AST_TYPE type, int line_number, int start_char, int end_char);


void add_node(struct NODE *parent, struct NODE *new_node);


void remove_node(struct NODE *parent, struct NODE *child);


int count_node_children(struct NODE *node);


int recursive_count_node_children(struct NODE *node);


void free_tree(struct NODE *child);


struct ARG_DATA *create_arg_data(char *name, struct TYPE_DATA *type, int line_number);


struct FUNC_PROTOTYPE *create_func_prototype(struct NODE *func);

void free_func_prototype_list(struct FUNC_PROTOTYPE *func);

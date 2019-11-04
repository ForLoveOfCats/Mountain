#pragma once
#include <stdint.h>


#include "symbols.h"



struct NODE *first_module;
struct NODE *last_module;

struct NODE *current_parse_parent_node;

struct NODE_BOX *first_boxed_func;
struct NODE_BOX *last_boxed_func;


enum AST_TYPE {
	AST_MODULE,
	AST_BLOCK,
	AST_IF,
	AST_ELIF,
	AST_ELSE,
	AST_WHILE,
	AST_BREAK,
	AST_CONTINUE,
	AST_FUNC,
	AST_TEST,
	AST_ENUM,
	AST_STRUCT,
	AST_LET,
	AST_NAME,
	AST_GET,
	AST_CALL,
	AST_RETURN,
	AST_EXPRESSION,
	AST_LITERAL,
	AST_NEW,
	AST_FIELDVALUE,
	AST_FIELDGET,
	AST_ENUMGET,
	AST_OP,
	AST_UNOP,
	AST_NEGATE
};

enum OP_TYPE {
	OP_NONE,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_EQUALS,
	OP_TEST_EQUAL,
	OP_TEST_NOT_EQUAL,
	OP_TEST_GREATER,
	OP_TEST_GREATER_EQUAL,
	OP_TEST_LESS,
	OP_TEST_LESS_EQUAL
};

enum UNOP_TYPE {
	UNOP_NONE,
	UNOP_PERIOD,
	UNOP_INVERT,
	UNOP_ADDRESS_OF,
	UNOP_DEREFERENCE
};

enum LITERAL_TYPE {
	LITERAL_I32,
	LITERAL_U8,
	LITERAL_BOOL
};


struct NODE
{
	enum AST_TYPE node_type;

	struct NODE *module;
	struct NODE *parent;
	struct NODE *next;
	struct NODE *previous;
	struct NODE *first_child;
	struct NODE *last_child;

	struct NODE *first_func;
	struct NODE *last_func;

	struct NODE *first_enum;
	struct NODE *last_enum;

	struct NODE *first_struct;
	struct NODE *last_struct;

	struct IMPORT_DATA *first_import;
	struct IMPORT_DATA *last_import;

	struct ARG_DATA *first_arg; //For function definitions (not calls)
	struct ARG_DATA *last_arg;

	int file;
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


struct IMPORT_DATA
{
	char *name;

	bool is_using;

	int file;
	int line_number;

	struct IMPORT_DATA *next;
};


struct NODE_BOX
{
	struct NODE *node; //The actual AST_FUNC node
	struct NODE_BOX *next;
};


struct NODE *lookup_module(char *name);


struct IMPORT_DATA *create_import_data(char *name, bool is_using, int file, int line_number);


void free_import_data(struct IMPORT_DATA *import_data);


struct NODE *create_node(enum AST_TYPE type, struct NODE *module, int file, int line_number, int start_char, int end_char);


void add_node(struct NODE *parent, struct NODE *new_node);


void remove_node(struct NODE *parent, struct NODE *child);


int count_node_children(struct NODE *node);


int recursive_count_node_children(struct NODE *node);


void free_tree(struct NODE *child);


struct ARG_DATA *create_arg_data(char *name, struct TYPE_DATA *type, int file, int line_number);


struct NODE_BOX *create_node_box(struct NODE *func);


void free_node_box_list(struct NODE_BOX *func);

#pragma once
#include <stdint.h>


enum AST_TYPE {AST_ROOT, AST_DEF};

struct NODE
{
	enum AST_TYPE type;

	struct NODE *parent;
	struct NODE *next_node;
	struct NODE *first_child;

	uint32_t stack_len;
	uint32_t stack_location; //Where in the stack the var is if def or get
};


struct NODE create_node(enum AST_TYPE);


void add_node(struct NODE new_node);


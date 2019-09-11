#pragma once
#include <stdio.h>

#include "ast.h"



void prepare_file(FILE *target);


void transpile_global_enums(FILE *target, struct NODE *root);


void prototype_globals(FILE *target, struct NODE *root);


void transpile_global_sets(FILE *target, struct NODE *root);


void prototype_functions(FILE *target);


void transpile_functions(FILE *target);


void transpile_tests(FILE *target, struct NODE *module);


void transpile_test_calls(FILE *target);


void transpile_block(FILE *target, struct NODE *node, int level);

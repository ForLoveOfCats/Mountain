#pragma once
#include <stdio.h>

#include "ast.h"



void prepare_file(FILE *target);


void prototype_globals(FILE *target, struct NODE *root);


void prototype_functions(FILE *target);


void transpile_functions(FILE *target);


void transpile_block(FILE *target, struct NODE *node, int level);

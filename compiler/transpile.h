#pragma once
#include <stdio.h>

#include "ast.h"


void transpile_block(FILE *target, struct NODE *node, int level);

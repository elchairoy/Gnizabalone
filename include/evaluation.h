#ifndef DF067538_D9C3_4DB6_9FA4_0090105C00B0
#define DF067538_D9C3_4DB6_9FA4_0090105C00B0
#include <stdio.h>

#include "board_struct.h"
#include "useful_functions.h"


#define MAX_WEIGHT 10000
#define STARTING_MARBLES_WHITE 14
#define STARTING_MARBLES_BLACK 14

#define WEIGHT_COUNT 324  // Adjust this based on your model size
#define FEATURE_COUNT 12

double evaluate(board *b, char color);

#endif /* DF067538_D9C3_4DB6_9FA4_0090105C00B0 */

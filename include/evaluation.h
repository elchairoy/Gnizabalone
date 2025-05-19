#ifndef DF067538_D9C3_4DB6_9FA4_0090105C00B0
#define DF067538_D9C3_4DB6_9FA4_0090105C00B0
#include <stdio.h>

#include "board_struct.h"
#include "useful_functions.h"


#define MAX_WEIGHT 10000
#define STARTING_MARBLES_WHITE 14
#define STARTING_MARBLES_BLACK 14


/* This function evaluates the position only by the points of the marbs. */
double evaluate(board *b, char color);
char get_modus(double center_proximity, double cohesion);
double lightweight_evaluate(board* b, char color);
void get_features(board* b, char color, double* features);

#endif /* DF067538_D9C3_4DB6_9FA4_0090105C00B0 */

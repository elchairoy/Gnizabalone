/*
 Created by meir on 29/11/2021.
 This is an Abalobe game engine.
*/

#ifndef DD0FAEFF_39F7_47FA_AB64_C1E01E82D91A
#define DD0FAEFF_39F7_47FA_AB64_C1E01E82D91A


#include <math.h>


#define RADIUS 5
#define NUMBER_OF_SQUARES 61


#define is_white(marb) (marb == white_marble) /* Check if the marb is white. */
#define is_black(marb) (marb == black_marble) /* Check if the marb is black. */


#define MAX_POSSIBLE_MOVES 300 /* Maximum number of moves possible in a position. */

enum types {empty, black_marble, white_marble}; /* The types of marbs. */

typedef unsigned int move;

enum node_type {PV_NODE, CUT_NODE, ALL_NODE}; /* The type of a node. */

typedef unsigned int irreversible_move_info; 

/* The struct of a position. */
typedef struct {
    char grid[2*RADIUS-1][2*RADIUS-1]; /* List of all the squares in the board.
                                                        Each square represented by half a char. */
    unsigned whose_turn : 1; /* 1 if it's white turn, 0 if it's black turn. */
}board;

#define get_in_bounds(r,c) (((r) > -RADIUS && (r) < RADIUS) && ((c) > -RADIUS && (c) < RADIUS) && ((r) + (c) > -RADIUS && (r) + (c) < RADIUS)) /* Check if the square is in bounds (hexagon). */

enum directions {LEFT, UP_LEFT, UP_RIGHT, RIGHT, DOWN_RIGHT, DOWN_LEFT}; /* The directions of the marbs. */

    

#define go_to_square(b,r,c) ((b)->grid[(r)+RADIUS-1][(c)+RADIUS-1]) /* Get the marb in the square. */
#define change_the_square(b,r,c,m) ((b)->grid[(r)+RADIUS-1][(c)+RADIUS-1] = (m)) /* Change the marb in the square. */

#define get_is_left_right_of(r1, c1, r2, c2) ((r1) == (r2))
#define get_is_up_right_down_left_of(r1, c1, r2, c2) ((c1) == (c2))
#define get_is_up_left_down_right_of(r1, c1, r2, c2) ((r1) + (c1) == (r2) + (c2))

#define get_is_in_line(r1, c1, r2, c2) \
    (get_is_left_right_of((r1), (c1), (r2), (c2)) || \
    get_is_up_right_down_left_of((r1), (c1), (r2), (c2)) || \
    get_is_up_left_down_right_of((r1), (c1), (r2), (c2))) /* Check if the two squares are in line. */

/* The struct of a game: */
typedef struct {
    board *current_position; /* The current position of the game. */
    board initial_position; /* The initial position of the game. */
    move moves[1000]; /* List of all the moves in the game. */
    int number_of_moves_in_game; /* Number of moves in the game. */
    char result; /* The result of the game. */
}game;


#endif /* DD0FAEFF_39F7_47FA_AB64_C1E01E82D91A */

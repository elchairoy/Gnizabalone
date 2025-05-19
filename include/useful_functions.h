#ifndef AE5BDBF7_77C4_4AB2_867A_1994FFAC6C77
#define AE5BDBF7_77C4_4AB2_867A_1994FFAC6C77

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>  

#include "possible_moves.h"
#include "board_struct.h"
#include "make_move.h"


#define MASK_FOR_A_HALF 0x0f /* A mask to get only a half of a byte. */
#define MASK_FOR_6BITS 0x003f /* A mask to get only 6 bits of a short. */
#define MASK_FOR_2BITS 0x0003 /* A mask to get only 2 bits of a short. */
#define MASK_FOR_3BITS 0x0007 /* A mask to get only 3 bits of a short. */
#define MASK_FOR_4BITS 0x000f /* A mask to get only 4 bits of a short. */

#define SRC_ROW_INDEX 0 /* The place of the row of the source square in the short. */
#define SRC_COL_INDEX 4 /* The place of the column of the source square in the short. */
#define DIRECTION_INDEX 8 /* The place of the direction of the move in the short. */
#define END_OF_LINE_ROW_INDEX 12 /* The place of the end of line row in the short. NOTE the end of line is only for the ASIDE moves. */
#define END_OF_LINE_COL_INDEX 16 /* The place of the end of line column in the short. NOTE the end of line is only for the ASIDE moves. */

#define NO_PUSHED_INDEX 0 /* The place of the pushed marbles in the short. */

/* This macro gets a move and returns the src square */
#define get_src_row(m) (((m & (MASK_FOR_4BITS << SRC_ROW_INDEX)) >> SRC_ROW_INDEX) - RADIUS + 1) /* Get the row of the source square. */
#define get_src_col(m) (((m & (MASK_FOR_4BITS << SRC_COL_INDEX)) >> SRC_COL_INDEX) - RADIUS + 1) /* Get the column of the source square. */
#define get_direction(m) ((m & (MASK_FOR_3BITS << DIRECTION_INDEX)) >> DIRECTION_INDEX) /* Get the direction of the move. */
#define get_end_of_line_row(m) (((m & (MASK_FOR_4BITS << END_OF_LINE_ROW_INDEX)) >> END_OF_LINE_ROW_INDEX) - RADIUS + 1) /* Get the end of line row. */
#define get_end_of_line_col(m) (((m & (MASK_FOR_4BITS << END_OF_LINE_COL_INDEX)) >> END_OF_LINE_COL_INDEX) - RADIUS + 1) /* Get the end of line column. */

enum move_types {STRAIGHT, ASIDE};

#define get_move_type(m) (get_end_of_line_row(m) == get_src_row(m) && get_end_of_line_col(m) == get_src_col(m) ? STRAIGHT : ASIDE) /* Get the type of the move. */

/* This macro gets an irreversible move info and returns the marb taken in the move */
#define get_no_pushed(move_info) ((move_info & (MASK_FOR_3BITS << NO_PUSHED_INDEX)) >> NO_PUSHED_INDEX)

/* This macro gets a move and returns if the move is a push */
#define get_is_push(move_info) (get_no_pushed(move_info) != 0)


/* This macro gets the data needed for a move and returns a short representing the move.*/
#define create_a_move(the_move, src_row, src_col, direction, end_of_line_row, end_of_line_col) \
    (the_move) = 0; \
    (the_move) |= ((src_row + RADIUS - 1) << SRC_ROW_INDEX); \
    (the_move) |= ((src_col + RADIUS - 1) << SRC_COL_INDEX); \
    (the_move) |= ((direction) << DIRECTION_INDEX); \
    (the_move) |= ((end_of_line_row + RADIUS - 1) << END_OF_LINE_ROW_INDEX); \
    (the_move) |= ((end_of_line_col + RADIUS - 1) << END_OF_LINE_COL_INDEX); 


/* This macro gets the data needed for a move and returns a short representing the move.*/
#define create_an_irrev_move_info(the_move_info, no_pushed) \
    (the_move_info) = 0; \
    (the_move_info) |= ((no_pushed) << NO_PUSHED_INDEX);


// MAX macro
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define get_enemy_marble(marb) (!(marb - 1) + 1)

#define get_marb_in_square(b,r,c) (get_in_bounds((r), (c)) ? go_to_square((b), (r), (c)) : -1) /* Get the marb in the square. */

#define get_marb_in_direction(b,r,c,d,n) \
    ((d) == LEFT ? get_marb_in_square((b), (r), (c)-(n)) : \
    ((d) == RIGHT ? get_marb_in_square((b), (r), (c)+(n)) : \
    ((d) == UP_LEFT ? get_marb_in_square((b), (r)+(n), (c)-(n)) : \
    ((d) == UP_RIGHT ? get_marb_in_square((b), (r)+(n), (c)) : \
    ((d) == DOWN_LEFT ? get_marb_in_square((b), (r)-(n), (c)) : \
    ((d) == DOWN_RIGHT ? get_marb_in_square((b), (r)-(n), (c)+(n)) : \
    -1))))))

#define change_the_square_in_direction(b,r,c,d,n,m) \
    ((d) == LEFT ? change_the_square((b), (r), (c)-(n), (m)) : \
    ((d) == RIGHT ? change_the_square((b), (r), (c)+(n), (m)) : \
    ((d) == UP_LEFT ? change_the_square((b), (r)+(n), (c)-(n), (m)) : \
    ((d) == UP_RIGHT ? change_the_square((b), (r)+(n), (c), (m)) : \
    ((d) == DOWN_LEFT ? change_the_square((b), (r)-(n), (c), (m)) : \
    ((d) == DOWN_RIGHT ? change_the_square((b), (r)-(n), (c)+(n), (m)) : \
    -1))))))

#define get_is_out_of_bounds_in_direction(r,c,d) \
    ((d) == LEFT ? get_in_bounds((r), (c)-1) : \
    ((d) == RIGHT ? get_in_bounds((r), (c)+1) : \
    ((d) == UP_LEFT ? get_in_bounds((r)+1, (c)-1) : \
    ((d) == UP_RIGHT ? get_in_bounds((r)+1, (c)) : \
    ((d) == DOWN_LEFT ? get_in_bounds((r)-1, (c)) : \
    ((d) == DOWN_RIGHT ? get_in_bounds((r)-1, (c)+1) : \
    -1))))))


#define get_backward_direction(d) \
    ((d) == LEFT ? RIGHT : \
    ((d) == RIGHT ? LEFT : \
    ((d) == UP_LEFT ? DOWN_RIGHT : \
    ((d) == UP_RIGHT ? DOWN_LEFT : \
    ((d) == DOWN_LEFT ? UP_RIGHT : \
    ((d) == DOWN_RIGHT ? UP_LEFT : \
    -1))))))

#define python_like_defining(a,b,x,y) \
    (a) = (x); \
    (b) = (y); 


#define hex_distance(q1, r1, q2, r2) (fmax(fmax(abs((q2) - (q1)), abs((r2) - (r1))), abs(-((q2) - (q1)) - ((r2) - (r1)))))

#define get_no_neighbours(b,r,c,color) \
    ((get_marb_in_square((b), (r)+1, (c)-1) == ((color) + 1)) + \
    (get_marb_in_square((b), (r)+1, (c)) == ((color) + 1)) + \
    (get_marb_in_square((b), (r), (c)-1) == ((color) + 1)) + \
    (get_marb_in_square((b), (r), (c)+1) == ((color) + 1)) + \
    (get_marb_in_square((b), (r)-1, (c)) == ((color) + 1)) + \
    (get_marb_in_square((b), (r)-1, (c)+1) == ((color) + 1))) /* Get the number of neighbours of the square. */

#define get_new_cords_in_direction(r,c,d,no_times_in_direction) \
    ((d) == LEFT ? (r) = (r), (c) = (c)-(no_times_in_direction) : \
    ((d) == RIGHT ? (r) = (r), (c) = (c)+(no_times_in_direction) : \
    ((d) == UP_LEFT ? (r) = (r)+(no_times_in_direction), (c) = (c)-(no_times_in_direction) : \
    ((d) == UP_RIGHT ? (r) = (r)+(no_times_in_direction), (c) = (c) : \
    ((d) == DOWN_LEFT ? (r) = (r)-(no_times_in_direction), (c) = (c) : \
    ((d) == DOWN_RIGHT ? (r) = (r)-(no_times_in_direction), (c) = (c)+(no_times_in_direction) : \
    -1))))))


char get_direction_between_squares(char src_row, char src_col, char dest_row, char dest_col);

char compare_boards(board *board1, board *board2);


void print_board(board *the_board);

void print_move(move the_move);

char *strrev(char *str);

char is_in_array(char *array, char value);

irreversible_move_info get_irrev_move_info(board *b, move m);

void unmake_move_in_board(board *b, move m, irreversible_move_info inf);

void unmake_move_in_game(game *the_game, move m, irreversible_move_info inf);

char check_repetition(game *the_game);

void selection_sort_for_moves(move moves[MAX_POSSIBLE_MOVES / 2], int *values, int k);

int remove_line_duplicates(char (*arr)[4], int n);

const char* coord_to_label(int x, int y);

char is_lost(board *b, char color);

char* label_to_coord(char *label);

double center_helping_score(board *b, move m);

double cohesion_helping_score(board *b, move m);

char push_move_score(board *b, move m);

double get_random_up_to_one();

char *my_strndup(const char *s, size_t n);

static inline double clamp(double x, double low, double high);

#endif /* AE5BDBF7_77C4_4AB2_867A_1994FFAC6C77 */

#ifndef AE5BDBF7_77C4_4AB2_867A_1994FFAC6C77
#define AE5BDBF7_77C4_4AB2_867A_1994FFAC6C77

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>  

#include "possible_moves.h"
#include "board_struct.h"
#include "make_move.h"
#include "evaluation.h"


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


// MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))

// MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))

// Use this once in a .c file to define it, extern elsewhere
// Use this once in a .c file to define it, extern elsewhere
extern char bounds_grid[11][11];

// === Macros ===

#define get_in_bounds(r, c) \
    (bounds_grid[(r) + RADIUS][(c) + RADIUS])

#define get_enemy_marble(marb) \
    (((marb) == 1) ? 2 : ((marb) == 2 ? 1 : -1))

static const char move_dr[6] = { 0, +1, +1, 0, -1, -1 }; 
static const char move_dc[6] = { -1, -1, 0, +1, +1, 0 };

#define get_marb_in_square(b, r, c) \
    (get_in_bounds(r, c) ? go_to_square((b), (r), (c)) : -1)

#define get_marb_dir1(b, r, c, d) \
    (get_in_bounds((r) + move_dr[(d)], (c) + move_dc[(d)]) \
        ? go_to_square((b), (r) + move_dr[(d)], (c) + move_dc[(d)]) \
        : -1)

#define get_marb_dir(b, r, c, d, n) \
    (get_in_bounds((r) + move_dr[(d)] * (n), (c) + move_dc[(d)] * (n)) \
        ? go_to_square((b), (r) + move_dr[(d)] * (n), (c) + move_dc[(d)] * (n)) \
        : -1)

#define change_the_square_in_direction(b, r, c, d, n, m) \
    change_the_square((b), (r) + move_dr[(d)] * (n), (c) + move_dc[(d)] * (n), (m))

#define get_is_out_of_bounds_in_direction(r, c, d) \
    get_in_bounds((r) + move_dr[(d)], (c) + move_dc[(d)])

#define get_backward_direction(d) \
    (((d) < 3) ? ((d) + 3) : ((d) - 3))

#define hex_distance(q1, r1, q2, r2) ({ \
    char dq = (q2) - (q1); \
    char dr = (r2) - (r1); \
    fmax(fmax(abs(dq), abs(dr)), abs(-dq - dr)); \
})

#define distance_of_line_from_square(q1, r1, d, q2, r2) ({ \
    char __dist; \
    switch (d) { \
        case LEFT: case RIGHT: \
            __dist = abs((q1) - (q2)); break; \
        case UP_LEFT: case DOWN_RIGHT: \
            __dist = abs(((q1) + (r1)) - ((q2) + (r2))); break; \
        case UP_RIGHT: case DOWN_LEFT: \
            __dist = abs((r1) - (r2)); break; \
        default: \
            __dist = -1; \
    } \
    __dist; \
})

#define get_no_neighbours(b, r, c, color) ({ \
    char __cnt = 0; \
    for (char __d = 0; __d < 6; __d++) \
        __cnt += (get_marb_in_square((b), (r) + move_dr[__d], (c) + move_dc[__d]) == (color) + 1); \
    __cnt; \
})

#define get_new_cords_in_direction(r, c, d, steps) { \
    *(r) += move_dr[(d)] * (steps); \
    *(c) += move_dc[(d)] * (steps); \
}




char get_direction_between_squares(char src_row, char src_col, char dest_row, char dest_col);

char compare_boards(board *board1, board *board2);


void print_board(board *the_board);

void print_move(move the_move);

irreversible_move_info get_irrev_move_info(board *b, move m);

void unmake_move_in_board(board *the_board, move m, irreversible_move_info inf);

void unmake_move_in_game(game *the_game, move m, irreversible_move_info inf);

char check_repetition(game *the_game, char is_in_search);

void selection_sort_for_moves(move moves[MAX_POSSIBLE_MOVES / 2], double *values, int k);

const char* cord_to_label(int x, int y);

char is_lost(board *b, char color);

void label_to_cord(char *label, char cord[2]);

double center_helping_score(board *b, move m);

char push_move_score(board *b, move m);

int three_in_a_row_helping_score(board *b, move m);

double get_random(double value);

char *my_strndup(const char *s, size_t n);

static inline double clamp(double x, double low, double high);

void get_board_string(board *b, char *str);

void print_board_string(board *b);

char is_capture(board *b, move m);

#endif /* AE5BDBF7_77C4_4AB2_867A_1994FFAC6C77 */


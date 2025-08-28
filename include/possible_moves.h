#ifndef DA695DC0_90E2_40B5_BC05_F55A01BC0605
#define DA695DC0_90E2_40B5_BC05_F55A01BC0605


#include "board_struct.h"
#include "useful_functions.h"
#include "make_move.h"

#include <stdio.h>

#define END 0 /*sign for the end of the array*/
#define WHITE 1 /*white is true*/
#define BLACK 0 /*black is false (I know it's Racist but I don't care so live with it)*/
#define MAX_PUSHED_ALLY 3 /*max number of pushed marbles*/
#define MAX_PUSHED_ENEMY 2 /*max number of pushed marbles*/
#define MAX_LINE_LENGTH 3 /* max number of marbles that can move in one ply */
#define MAX_LINES_IN_BOARD 200 /* max number of lines in the board */

/*main func*/
void get_possible_moves(board *the_board,move *new_all_moves,move *all_moves_last_move, move last_move, irreversible_move_info inf);
void get_all_moves_by_calculating_everything(board *the_board, move *all_moves);
void get_possible_moves(board *the_board,move *new_all_moves,move *all_moves_last_move, move last_move, irreversible_move_info inf);


#endif /* DA695DC0_90E2_40B5_BC05_F55A01BC0605 */

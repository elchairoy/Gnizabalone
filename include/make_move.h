#ifndef EE025185_6380_4B7E_AEF0_4F6D62B6980A
#define EE025185_6380_4B7E_AEF0_4F6D62B6980A
#include <stdio.h>

#include "board_struct.h"
#include "useful_functions.h"
#include "possible_moves.h"

/* A function that receives a move and executes it. (For games) */
void commit_a_move_in_game(game *the_game, move m);

/* A function that receives a move and executes it. (For boards) */
void commit_a_move_in_board(board *the_board, move m);



#endif /* EE025185_6380_4B7E_AEF0_4F6D62B6980A */

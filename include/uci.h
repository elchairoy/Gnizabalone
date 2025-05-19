#ifndef AFD0BF27_C006_46B4_89E6_171B4DC00121
#define AFD0BF27_C006_46B4_89E6_171B4DC00121

#include <time.h>
#include <math.h>
#include "string.h"
#include "board_struct.h"
#include "possible_moves.h"
#include "useful_functions.h"
#include "make_move.h"
#include "minimax.h"
#include "evaluation.h"
#include "fine_tune_eval.h"


#define MAX_DEPTH 7

#define IS_HT_SEARCH 1
#define IS_PV_SEARCH 1



char fen_to_board(char *fen, board *b); 
double bot_move(game *the_game, HashTable *ht, char logs);
int check_endgame(game *the_game);
char translate_promotion(char promotion);
int check_src(board *the_board, char src);
char uci_parse(game *the_game, char is_game_on, HashTable *ht);
int main();
int player_move(game *the_game, char *str);
char translate_marb_symbol(char marb_symbol);
void moves_in_depth(char d,board *b,move *all_moves_last_move, move last_move, irreversible_move_info inf);
void create_game(game *g, board *initial_position);
double simulate(HashTable *ht, char depth, Dataset *ds);
void init_empty_board(board *b);
void begian_daisy_opening(board *b);
double who_won(game *the_game);


#endif /* AFD0BF27_C006_46B4_89E6_171B4DC00121 */

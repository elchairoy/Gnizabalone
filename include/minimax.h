#ifndef BE5F1FF0_4F72_47C7_8DFE_407829A022EA
#define BE5F1FF0_4F72_47C7_8DFE_407829A022EA

#include "board_struct.h"
#include "useful_functions.h"
#include "make_move.h"
#include "possible_moves.h"
#include "evaluation.h"
#include "hashtable.h"
#include "uci.h"

#define create_a_minimax_eval(minimax_eval ,evaluation, type_of_node) minimax_eval.eval = (evaluation); minimax_eval.type = (type_of_node);
#define create_a_minimax_move_eval(minimax_eval ,evaluation, type_of_node, move) minimax_eval.eval = (evaluation); minimax_eval.type = (type_of_node); minimax_eval.m = (move);

typedef struct {
    double eval;
    enum node_type type;
    move m;
} minimax_eval;

minimax_eval evaluate_minimax_for_white(game *b, char depth ,double alpha, double beta, HashTable *ht);
minimax_eval evaluate_minimax_for_black(game *b, char depth, double alpha, double beta, HashTable *ht);
minimax_eval get_best_move_white(game *b,char depth, double alpha, double beta, HashTable *ht);
minimax_eval get_best_move_black(game *b,char depth, double alpha, double beta, HashTable *ht);

#define ASPIRATION_WINDOW 0.0005 /* The aspiration window for the alpha-beta pruning. */


void order_moves(game *g, move *all_moves, int *move_values, char depth, HashTable *ht);
void order_noisy_moves(board *b, move *noisy_moves, int *move_values, HashTable *ht);

int SEE(board *the_board, move *all_moves, char square);
int SEE_for_move(board *the_board, move *all_moves, move m);

void decay_history_heuristic(double decay);

#define HISTORY_DECAY 0.9

#endif /* BE5F1FF0_4F72_47C7_8DFE_407829A022EA */

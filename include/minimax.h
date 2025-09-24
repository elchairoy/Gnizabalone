#ifndef BE5F1FF0_4F72_47C7_8DFE_407829A022EA
#define BE5F1FF0_4F72_47C7_8DFE_407829A022EA

#include "board_struct.h"
#include "useful_functions.h"
#include "make_move.h"
#include "possible_moves.h"
#include "evaluation.h"
#include "hashtable.h"
#include "uci.h"

typedef struct {
    double eval;
    enum node_type type;
    move m;
} minimax_eval;

static inline void create_a_minimax_eval(minimax_eval *minimax_eval ,double evaluation, enum node_type type_of_node) {minimax_eval->eval = (evaluation); minimax_eval->type = (type_of_node);}
static inline void create_a_minimax_move_eval(minimax_eval *minimax_eval ,double evaluation, enum node_type type_of_node, move m) {minimax_eval->eval = (evaluation); minimax_eval->type = (type_of_node); minimax_eval->m = (m);}


minimax_eval quiescence_search_white(game *the_game, double alpha, double beta, char depth, HashTable *ht);
minimax_eval quiescence_search_black(game *the_game, double alpha, double beta, char depth, HashTable *ht);
minimax_eval evaluate_minimax_for_white(game *the_game, char depth ,double alpha, double beta, HashTable *ht);
minimax_eval evaluate_minimax_for_black(game *the_game, char depth, double alpha, double beta, HashTable *ht);
minimax_eval get_best_move_white(game *the_game,char depth, double alpha, double beta, HashTable *ht);
minimax_eval get_best_move_black(game *the_game,char depth, double alpha, double beta, HashTable *ht);

void order_moves(game *g, move *all_moves, double *move_values, char depth, HashTable *ht);
void order_moves_queiscence(game *g, move *all_moves, double *move_values, HashTable *ht);

void decay_history_heuristic(double decay);

#define HISTORY_DECAY 0.9

#define MAX_EVAL 100 /* The maximum evaluation possible. */
#define MIN_EVAL -100 /* The minimum evaluation possible. */


#endif /* BE5F1FF0_4F72_47C7_8DFE_407829A022EA */

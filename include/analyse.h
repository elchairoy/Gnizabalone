#ifndef BE5F1FF0_4F72_47C7_8DFE_407829D022EA
#define BE5F1FF0_4F72_47C7_8DFE_407829D022EA

#include "board_struct.h"
#include "useful_functions.h"
#include "make_move.h"
#include "possible_moves.h"
#include "minimax.h"
#include "hashtable.h"
#include "uci.h"

#define MAX_PV_LENGTH 64

typedef struct {
    double eval;                // evaluation of this PV
    int length;                 // number of moves in PV
    move moves[MAX_PV_LENGTH];  // moves in the PV
} pv_line_t;

int get_best_pvs_white(game *the_game, char depth, double alpha, double beta,
                       HashTable *ht, int num_pvs, pv_line_t *pv_results);
int get_best_pvs_black(game *the_game, char depth, double alpha, double beta,
                       HashTable *ht, int num_pvs, pv_line_t *pv_results);


#endif /* BE5F1FF0_4F72_47C7_8DFE_407829D022EA */

#include "../include/minimax.h"

#define MAX_EVAL 100 /* The maximum evaluation possible. */
#define MIN_EVAL -100 /* The minimum evaluation possible. */

#define EVAL_TO_PRUNE 5 /* In what evaluation the minimax will cut the moves branch because it's too bad. */
#define RAZORING_MARGIN 0.2 /* The margin for razoring. */
char NULL_MOVE_REDUCTION; /* The null move reduction. */
char MIN_NULL_MOVE; /* The minimum depth for null move. */
char is_nms; /* If null move search is on. */
char is_quiescence; /* If quiescence search is on. */
char Q_DEPTH; /* The quiescence search depth. */
char is_ht_search; /* If hash table search is on. */

long int number_of_moves = 0; /* The number of positions scaned. */
long int number_of_ht_found = 0; /* The number of positions found in the hash table. */
long int number_of_ht_inserted = 0; /* The number of positions inserted to the hash table. */
long int number_of_null_moves = 0; /* The number of null moves. */


move killer_moves[10][2]; /* The killer moves. */
int history_heuristic_push[2*RADIUS - 1][2*RADIUS - 1][6]; /* The history heuristic for push moves. stores src and direction. */
int history_heuristic_aside[2*RADIUS - 1][2*RADIUS - 1][6][2*RADIUS - 1][2*RADIUS - 1]; /* The history heuristic for aside moves. stores src and direction and end of line. */


#define update_hh_aside(move) \
    history_heuristic_aside[get_src_row(move)+RADIUS-1][get_src_col(move)+RADIUS-1][get_direction(move)][get_end_of_line_row(move)+RADIUS-1][get_end_of_line_col(move)+RADIUS-1] += 1; \
    history_heuristic_aside[get_end_of_line_row(move)+RADIUS-1][get_end_of_line_col(move)+RADIUS-1][get_direction(move)][get_src_row(move)+RADIUS-1][get_src_col(move)+RADIUS-1] += 1;

#define update_hh_push(move) \
    history_heuristic_push[get_src_row(move)+RADIUS-1][get_src_col(move)+RADIUS-1][get_direction(move)] += 1; \

#define update_history_heuristic(move) \
    if (get_move_type(move) == ASIDE) { \
        update_hh_aside(move); \
    } else { \
        update_hh_push(move); \
    }

#define get_hh_aside(move) \
    history_heuristic_aside[get_src_row(move)+RADIUS-1][get_src_col(move)+RADIUS-1][get_direction(move)][get_end_of_line_row(move)+RADIUS-1][get_end_of_line_col(move)+RADIUS-1]

#define get_hh_push(move) \
    history_heuristic_push[get_src_row(move)+RADIUS-1][get_src_col(move)+RADIUS-1][get_direction(move)]

#define get_history_heuristic(move) \
    (get_move_type(move) == ASIDE ? get_hh_aside(move) : get_hh_push(move))

#define update_killer_moves(move) \
    if (killer_moves[depth][0] != move) { \
        killer_moves[depth][1] = killer_moves[depth][0]; \
        killer_moves[depth][0] = move; \
    } \



void decay_history_heuristic(double decay) {
    for (int i = 0; i < 2*RADIUS - 1; i++) {
        for (int j = 0; j < 2*RADIUS - 1; j++) {
            for (int k = 0; k < 6; k++) {
                history_heuristic_push[i][j][k] *= decay;
            }
        }
    }
    for (int i = 0; i < 2*RADIUS - 1; i++) {
        for (int j = 0; j < 2*RADIUS - 1; j++) {
            for (int k = 0; k < 6; k++) {
                for (int l = 0; l < 2*RADIUS - 1; l++) {
                    for (int m = 0; m < 2*RADIUS - 1; m++) {
                        history_heuristic_aside[i][j][k][l][m] *= decay;
                    }
                }
            }
        }
    }
}


clock_t start_time;
double max_duration = -1; /* The maximum duration of the search in seconds. -1 means no limit. */

// This function performs a quiescence search for the black player.
minimax_eval quiescence_search_black(game *the_game, double alpha, double beta, char depth, HashTable *ht) {
    board *b = the_game->current_position;
    move all_moves[MAX_POSSIBLE_MOVES];
    int i = 0;
    minimax_eval temp;
    const void *tempvoid;
    double move_values[MAX_POSSIBLE_MOVES];
    double best_eval = MAX_EVAL;
    // Evaluate the current position statically. This is the "stand-pat" value.
    if (is_lost(b, BLACK)) {
        create_a_minimax_eval(&temp, MAX_EVAL, PV_NODE);
        return temp;
    }

    if (check_repetition(the_game, 1)) { /* Check for repetition */
        create_a_minimax_eval(&temp, 0, PV_NODE);
        return temp;
    }

    if ((tempvoid = _ht_search_pos(ht, the_game, 0, PV_NODE)) != NULL) { /* Make sure this would also be cutted. */
        number_of_ht_found++;
        create_a_minimax_eval(&temp, ((ht_move_eval_struct *)tempvoid)->eval, PV_NODE);
        return (temp);
    }
    if ((tempvoid = _ht_search_pos(ht, the_game, 0, LOWERBOUND)) != NULL && ((ht_move_eval_struct *)tempvoid)->eval >= beta) { /* Make sure this would also be cutted. */
        number_of_ht_found++;
        create_a_minimax_eval(&temp, beta, FAIL_HIGH);
        return temp;
    }
    if ((tempvoid = _ht_search_pos(ht, the_game, 0, UPPERBOUND)) != NULL && ((ht_move_eval_struct *)tempvoid)->eval <= alpha) { /* Make sure this would also be cutted. */
        number_of_ht_found++;
        create_a_minimax_eval(&temp, alpha, FAIL_LOW);
        return temp;
    }
        

    double stand_pat_eval = get_random(evaluate(b, the_game->current_position->whose_turn));
    best_eval = stand_pat_eval;

    
    // If the stand-pat evaluation is already better than beta, we can prune
    // and return a cutoff value.
    if (stand_pat_eval < alpha) {
        create_a_minimax_eval(&temp, alpha, FAIL_LOW);
        return temp;
    }
    if (is_quiescence == 0 || depth <= 0) {
        if (stand_pat_eval <= beta)
            create_a_minimax_eval(&temp, stand_pat_eval, PV_NODE);
        else 
            create_a_minimax_eval(&temp, beta, FAIL_HIGH);
        return temp;
    }

    if (stand_pat_eval < beta) {
        beta = stand_pat_eval;
    }



    // Get all possible moves and filter them to only include captures.
    get_possible_moves(b, all_moves);
    create_a_minimax_eval(&temp, stand_pat_eval, PV_NODE);

    // Search through all capture moves.
    i = 0;
    order_moves_queiscence(the_game, all_moves, move_values, ht);
    while (all_moves[i] != END && i != 2) {
        selection_sort_for_moves(all_moves, move_values, i);
        if (move_values[i] < EVAL_TO_PRUNE) {
            break; // Prune moves that are too bad.
        }
        irreversible_move_info temp_inf = get_irrev_move_info(b, all_moves[i]);
        commit_a_move_in_game(the_game, all_moves[i]);
        
        // Recurse to the white player's quiescence search.
        temp = quiescence_search_white(the_game, alpha, beta, depth - 1, ht);
        
        unmake_move_in_game(the_game, all_moves[i], temp_inf);

        // Update beta if we find a better (lower) score.
        if (temp.eval < beta) {
            beta = temp.eval;
        }

        // Check for an alpha cutoff. If the minimizing player can force a position
        // that is worse for the maximizing player than a previously explored move,
        // we can prune this branch.
        if (beta <= alpha) {
            create_a_minimax_eval(&temp, alpha, FAIL_LOW);
            return temp;
        }

        if (best_eval > temp.eval) {
            best_eval = temp.eval;
        }
        i++;
    }
    create_a_minimax_eval(&temp, best_eval, PV_NODE);
    return temp;
}

// This function performs a quiescence search for the black player.
minimax_eval quiescence_search_white(game *the_game, double alpha, double beta, char depth, HashTable *ht) {
    board *b = the_game->current_position;
    move all_moves[MAX_POSSIBLE_MOVES];
    int i = 0;
    minimax_eval temp;
    const void *tempvoid;
    double move_values[MAX_POSSIBLE_MOVES];
    double best_eval = MIN_EVAL;
    // Evaluate the current position statically.
    if (is_lost(b, WHITE)) {
        create_a_minimax_eval(&temp, MIN_EVAL, PV_NODE);
        return temp;
    }

    if (check_repetition(the_game, 1))/* Repitition */ {
        create_a_minimax_eval(&temp, 0, PV_NODE);
        return temp;
    }

    if ((tempvoid = _ht_search_pos(ht, the_game, 0, PV_NODE)) != NULL) { /* Make sure this would also be cutted. */
        number_of_ht_found++;
        create_a_minimax_eval(&temp, ((ht_move_eval_struct *)tempvoid)->eval, PV_NODE);
        return (temp);
    }
    if ((tempvoid = _ht_search_pos(ht, the_game, 0, LOWERBOUND)) != NULL && ((ht_move_eval_struct *)tempvoid)->eval >= beta) { /* Make sure this would also be cutted. */
        number_of_ht_found++;
        create_a_minimax_eval(&temp, beta, FAIL_HIGH);
        return temp;
    }
    if ((tempvoid = _ht_search_pos(ht, the_game, 0, UPPERBOUND)) != NULL && ((ht_move_eval_struct *)tempvoid)->eval <= alpha) { /* Make sure this would also be cutted. */
        number_of_ht_found++;
        create_a_minimax_eval(&temp, alpha, FAIL_LOW);
        return temp;
    }


    double stand_pat_eval = get_random(evaluate(b, the_game->current_position->whose_turn));
    best_eval = stand_pat_eval;

    if (stand_pat_eval > beta) {
        create_a_minimax_eval(&temp, beta, FAIL_HIGH);
        return temp;
    }
    

    if (is_quiescence == 0 || depth <= 0) {
        if (stand_pat_eval >= alpha)
            create_a_minimax_eval(&temp, stand_pat_eval, PV_NODE);
        else 
            create_a_minimax_eval(&temp, alpha, FAIL_LOW);
        return temp;
    }


    if (stand_pat_eval > alpha) {
        alpha = stand_pat_eval;
    }

    // Get all possible moves and filter them to only include captures.
    get_possible_moves(b, all_moves);
    create_a_minimax_eval(&temp, stand_pat_eval, PV_NODE);

    // Search through all capture moves.
    i = 0;
    
    order_moves_queiscence(the_game, all_moves, move_values, ht);
    while (all_moves[i] != END && i != 2) {
        selection_sort_for_moves(all_moves, move_values, i);
        
        
        if (move_values[i] < EVAL_TO_PRUNE) {
            break; // Prune moves that are too bad.
        }
        irreversible_move_info temp_inf = get_irrev_move_info(b, all_moves[i]);
        commit_a_move_in_game(the_game, all_moves[i]);
        
        // Recurse to the black player's quiescence search.
        temp = quiescence_search_black(the_game, alpha, beta, depth - 1, ht);
        
        unmake_move_in_game(the_game, all_moves[i], temp_inf);

        // Update alpha if we find a better (higher) score.
        if (temp.eval > alpha) {
            alpha = temp.eval;
        }

        // Check for a beta cutoff. If the maximizing player can force a position
        // that is better than the beta value, we can prune this branch.
        if (alpha >= beta) {
            create_a_minimax_eval(&temp, beta, FAIL_HIGH);
            return temp;
        }

        if (best_eval < temp.eval) {
            best_eval = temp.eval;
        }
        i++;
    }
    create_a_minimax_eval(&temp, best_eval, PV_NODE);
    return temp;
}


/* This function gets a board (when is white's move) and the depth and evaluates the position using minimax. */
minimax_eval evaluate_minimax_for_white(game *the_game, char depth, double alpha, double beta, HashTable *ht) {
    move all_moves[MAX_POSSIBLE_MOVES];
    int i = 0;
    minimax_eval temp;
    board *b = the_game->current_position;
    const void *tempvoid;
    move best;
    double max = MIN_EVAL;
    char is_pv_node = 0;
    number_of_moves++;
    double move_values[MAX_POSSIBLE_MOVES]; /* The values of the moves. */


    if (is_lost(b,WHITE)) {
        _ht_insert_pos(ht, the_game, depth, END, MIN_EVAL, PV_NODE);
        create_a_minimax_eval(&temp, MIN_EVAL, PV_NODE);
        return temp;
    }

    if (check_repetition(the_game, 1))/* Repitition */ {
        _ht_insert_pos(ht, the_game, depth, END, 0, PV_NODE);
        create_a_minimax_eval(&temp, 0, PV_NODE);
        return temp;
    }

    clock_t current_time = clock();
    if (max_duration != -1 && ((double)(current_time - start_time) / CLOCKS_PER_SEC) >= max_duration) {
        create_a_minimax_eval(&temp, 0, NO_TIME);
        return temp;
    }
    /* Search for the position in the hash table: */
    if ((tempvoid = _ht_search_pos(ht, the_game, depth, PV_NODE)) != NULL) { /* Make sure this would also be cutted. */
        number_of_ht_found++;
        create_a_minimax_eval(&temp, ((ht_move_eval_struct *)tempvoid)->eval, PV_NODE);
        return (temp);
    }
    if ((tempvoid = _ht_search_pos(ht, the_game, depth, UPPERBOUND)) != NULL && ((ht_move_eval_struct *)tempvoid)->eval <= alpha) { /* Make sure this would also be cutted. */
        number_of_ht_found++;
        create_a_minimax_eval(&temp, alpha, FAIL_LOW);
        return temp;
    }
    if ((tempvoid = _ht_search_pos(ht, the_game, depth, LOWERBOUND)) != NULL && ((ht_move_eval_struct *)tempvoid)->eval >= beta) { /* Make sure this would also be cutted. */
        number_of_ht_found++;
        create_a_minimax_eval(&temp, beta, FAIL_HIGH);
        update_killer_moves(((ht_move_eval_struct *)tempvoid)->best_move);
        update_history_heuristic(((ht_move_eval_struct *)tempvoid)->best_move);
        return temp;
    }

    if (depth == 0) {
        temp = quiescence_search_white(the_game, alpha, beta, Q_DEPTH, ht);
        _ht_insert_pos(ht, the_game, 0, END, temp.eval, temp.type);
        return temp;
    }

    get_possible_moves(b,all_moves); /* Gets all the moves possible. */
    best = all_moves[0]; /* The best move in the position. */

    /* NULL MOVE SEARCH */
    if (depth >= MIN_NULL_MOVE && is_nms) {
        number_of_null_moves++;
        is_nms = 0;
        b->whose_turn = BLACK;
        temp = evaluate_minimax_for_black(the_game, MAX(depth - NULL_MOVE_REDUCTION, 0), alpha, beta, ht);
        b->whose_turn = WHITE;
        is_nms = 1;
        if (temp.eval >= beta) {
            _ht_insert_pos(ht, the_game, depth, END, temp.eval, LOWERBOUND);
            create_a_minimax_eval(&temp, beta, FAIL_HIGH);
            return temp;
        }
    }

    order_moves(the_game, all_moves, move_values, depth, ht); /* Orders the moves. */
    while (all_moves[i] != END) {
        selection_sort_for_moves(all_moves, move_values, i); /* Sorts the moves. */
        irreversible_move_info temp_inf = get_irrev_move_info(b,all_moves[i]);
        commit_a_move_in_game(the_game,all_moves[i]); /* Commits the move. */
        temp = evaluate_minimax_for_black(the_game, depth - 1, alpha, beta, ht);
        unmake_move_in_game(the_game,all_moves[i],temp_inf);
        if (temp.type == NO_TIME) {
            return temp;
        }
        if (temp.eval > alpha) {
            alpha = temp.eval;
            is_pv_node = 1;
        }
        if (temp.eval >= beta) {
            _ht_insert_pos(ht, the_game, depth, all_moves[i], temp.eval, LOWERBOUND);
            create_a_minimax_eval(&temp, beta, FAIL_HIGH);
            update_killer_moves(all_moves[i]);
            update_history_heuristic(all_moves[i]);
            return temp;
        }
        if (temp.eval > max) {
            max = temp.eval;
            best = all_moves[i];
            
            if (temp.eval >= MAX_EVAL) {
                _ht_insert_pos(ht, the_game, depth, all_moves[i], MAX_EVAL, PV_NODE);
                create_a_minimax_eval(&temp, MAX_EVAL, PV_NODE);
                return temp;
            }
        }
        i++;
    }
    if (is_pv_node) {
        _ht_insert_pos(ht, the_game, depth, best, max, PV_NODE);
        create_a_minimax_eval(&temp, max, PV_NODE);
    }
    else {
        _ht_insert_pos(ht, the_game, depth, END, alpha, UPPERBOUND);
        create_a_minimax_eval(&temp, alpha, FAIL_LOW);
    }
    return temp;
}
/* This function gets a board (when is black's move) and the depth and evaluates the position using minimax. */
minimax_eval evaluate_minimax_for_black(game *the_game, char depth, double alpha, double beta, HashTable *ht) {
    board *b = the_game->current_position;
    move all_moves[MAX_POSSIBLE_MOVES];
    int i = 0;
    minimax_eval temp;
    const void *tempvoid;
    move best;
    double min = MAX_EVAL;
    char is_pv_node = 0;
    number_of_moves++;
    double move_values[MAX_POSSIBLE_MOVES]; /* The values of the moves. */
    
    
    if (is_lost(b,BLACK)) {
        _ht_insert_pos(ht, the_game, depth, END, MAX_EVAL, PV_NODE);
        create_a_minimax_eval(&temp, MAX_EVAL, PV_NODE);
        return temp;
    }

    if (check_repetition(the_game, 1)) {
        _ht_insert_pos(ht, the_game, depth, END, 0, PV_NODE);
        create_a_minimax_eval(&temp, 0, PV_NODE);
        return temp;
    }

    clock_t current_time = clock();
    if (max_duration != -1 && ((double)(current_time - start_time) / CLOCKS_PER_SEC) >= max_duration) {
        create_a_minimax_eval(&temp, 0, NO_TIME);
        return temp;
    }  

        

    /* Search for the position in the hash table: */
    if ((tempvoid = _ht_search_pos(ht, the_game, depth, PV_NODE)) != NULL) { /* Make sure this would also be cutted. */
        number_of_ht_found++;
        create_a_minimax_eval(&temp, ((ht_move_eval_struct *)tempvoid)->eval, PV_NODE);
        return (temp);
    }
    if ((tempvoid = _ht_search_pos(ht, the_game, depth, UPPERBOUND)) != NULL && ((ht_move_eval_struct *)tempvoid)->eval <= alpha) { /* Make sure this would also be cutted. */
        number_of_ht_found++;
        create_a_minimax_eval(&temp, alpha, FAIL_LOW);
        update_killer_moves(((ht_move_eval_struct *)tempvoid)->best_move);
        update_history_heuristic(((ht_move_eval_struct *)tempvoid)->best_move);
        return temp;
    } 
    if ((tempvoid = _ht_search_pos(ht, the_game, depth, LOWERBOUND)) != NULL && ((ht_move_eval_struct *)tempvoid)->eval >= beta) { /* Make sure this would also be cutted. */
        number_of_ht_found++;
        create_a_minimax_eval(&temp, beta, FAIL_HIGH);
        return temp;
    }

    if (depth == 0) {
        temp = quiescence_search_black(the_game, alpha, beta, Q_DEPTH, ht);
        _ht_insert_pos(ht, the_game, 0, END, temp.eval, temp.type);
        return temp;
    }

    get_possible_moves(b,all_moves); /* Gets all the moves possible. */
    best = all_moves[0];


    /* NULL MOVE SEARCH */
    if (depth >= MIN_NULL_MOVE && is_nms) {
        number_of_null_moves++;
        is_nms = 0;
        b->whose_turn = WHITE;
        temp = evaluate_minimax_for_white(the_game, MAX(depth - NULL_MOVE_REDUCTION, 0), alpha, beta, ht);
        b->whose_turn = BLACK;
        is_nms = 1;
        if (temp.eval <= alpha) {
            _ht_insert_pos(ht, the_game, depth, END, temp.eval, UPPERBOUND);
            create_a_minimax_eval(&temp, alpha, FAIL_LOW);
            return temp;
        }
    }
    

    order_moves(the_game, all_moves, move_values, depth, ht); /* Orders the moves. */
    while (all_moves[i] != END) {
        selection_sort_for_moves(all_moves, move_values, i); /* Sorts the moves. */
        irreversible_move_info temp_inf = get_irrev_move_info(b,all_moves[i]);
        commit_a_move_in_game(the_game,all_moves[i]); /* Commits the move. */
        temp = evaluate_minimax_for_white(the_game, depth - 1, alpha, beta, ht);
        unmake_move_in_game(the_game,all_moves[i],temp_inf);
        if (temp.type == NO_TIME) {
            return temp;
        }
        if (temp.eval < beta) {
            beta = temp.eval;
            is_pv_node = 1;
            
        }
        if (temp.eval <= alpha) {
            _ht_insert_pos(ht, the_game, depth, all_moves[i], temp.eval, UPPERBOUND);
            create_a_minimax_eval(&temp, alpha, FAIL_LOW);
            update_killer_moves(all_moves[i]);
            update_history_heuristic(all_moves[i]);
            return temp;
        }
        if (temp.eval < min) {
            min = temp.eval;
            best = all_moves[i];
            
            if (temp.eval <= MIN_EVAL) {
                _ht_insert_pos(ht, the_game, depth, all_moves[i], MIN_EVAL, PV_NODE);
                create_a_minimax_eval(&temp, MIN_EVAL, PV_NODE);
                return temp;
            }                
        }
        i++;
    }
    
    if (is_pv_node) {
        _ht_insert_pos(ht, the_game, depth, best, min, PV_NODE);
        create_a_minimax_eval(&temp, min, PV_NODE);
    }
    else {
        _ht_insert_pos(ht, the_game, depth, END, beta, LOWERBOUND);
        create_a_minimax_eval(&temp, beta, FAIL_HIGH);
    }
    return temp;
}

extern char is_comparing;

/* The main function of minimax for white, returns the best move of the position. */
minimax_eval get_best_move_white(game *the_game, char depth, double alpha, double beta, HashTable *ht) {
    board *b = the_game->current_position;
    move all_moves[MAX_POSSIBLE_MOVES]; /* All the moves possible in the position. */
    int i = 0;
    double max = alpha; /* The maximun eval possible in the position(maximum = best for white). */
    minimax_eval temp;
    move best;
    number_of_moves++;
    const void *tempvoid;
    char is_pv_node = 0;
    double move_values[MAX_POSSIBLE_MOVES]; /* The values of the moves. */
    is_quiescence = 1;
    Q_DEPTH = 10;
    is_nms = 1;
    is_ht_search = 1; /* When comparing agents we dont want to use ht, so no data sharing. */

    
    
    get_possible_moves(b,all_moves); /* Gets all the moves possible. */
    best = all_moves[0]; /* The default move . */

    

    if (is_lost(b,WHITE)) {
        create_a_minimax_move_eval(&temp, MIN_EVAL, PV_NODE, END);
        return temp;
    }

    order_moves(the_game,all_moves,move_values, depth, ht); /* Orders the moves. */

    while (all_moves[i] != END) {
        selection_sort_for_moves(all_moves, move_values, i); /* Sorts the moves. */
        irreversible_move_info temp_inf = get_irrev_move_info(b,all_moves[i]);
        commit_a_move_in_game(the_game,all_moves[i]); /* Commits the move. */
        temp = evaluate_minimax_for_black(the_game, depth - 1, max, beta, ht); /* Checks what is the eval after the move. */
        unmake_move_in_game(the_game,all_moves[i],temp_inf);
        if (temp.type == NO_TIME) {
            return temp;
        }
        if (temp.eval > max) { /* If the eval is better then the max eval: */
            max = temp.eval; /* Changes max to be it. */
            best = all_moves[i];
            
            if (temp.eval >= MAX_EVAL) {
                is_pv_node = 1;
                break; /* If the eval is a mate, we can break. */
            }
        }
        if (temp.eval > alpha && temp.eval < beta) {
            is_pv_node = 1;
        }
        i++; 
    }
    if (!is_pv_node) {
        return get_best_move_white(the_game, depth, 2*MIN_EVAL, 2*MAX_EVAL, ht);
    }
    _ht_insert_pos(ht, the_game, depth, best, max, PV_NODE);
    create_a_minimax_move_eval(&temp, max, PV_NODE, best);
    return temp;
}

/* The main function of minimax for black, returns the best move of the position. */
minimax_eval get_best_move_black(game *the_game,char depth, double alpha, double beta, HashTable *ht) {
    board *b = the_game->current_position;
    move all_moves[MAX_POSSIBLE_MOVES]; /* All the moves possible in the position. */
    int i = 0;
    double min = beta; /* The maximun eval possible in the position(maximum = best for white). */
    move best; /* The best move in the position. */
    minimax_eval temp;
    number_of_moves++;
    char is_pv_node = 0;
    const void *tempvoid;
    double move_values[MAX_POSSIBLE_MOVES]; /* The values of the moves. */
    is_nms = 1;
    is_quiescence = 1; /* We are not in quiescence search. */
    Q_DEPTH = 10;
    is_ht_search = 1;

    
    get_possible_moves(b,all_moves); /* Gets all the moves possible. */
    best = all_moves[i]; /* The default move . */

    

    if (is_lost(b,BLACK)) {
        create_a_minimax_move_eval(&temp, MAX_EVAL, PV_NODE, END);
        return temp;
    }
    order_moves(the_game, all_moves, move_values, depth, ht);
    while (all_moves[i] != END) {
        /* Make a selection sort in all_moves, using the values of the moves: */
        selection_sort_for_moves(all_moves, move_values, i);
        irreversible_move_info temp_inf = get_irrev_move_info(b,all_moves[i]);
        commit_a_move_in_game(the_game,all_moves[i]); /* Commits the move. */
        temp = evaluate_minimax_for_white(the_game, depth - 1, alpha, min, ht); /* Checks what is the eval after the move. */
        unmake_move_in_game(the_game,all_moves[i],temp_inf);
        if (temp.type == NO_TIME) {
            return temp;
        }
        if (temp.eval < min) { /* If the eval is better then the max eval: */
            min = temp.eval; /* Changes max to be it. */
            best = all_moves[i];
            
            if (temp.eval <= MIN_EVAL) {
                is_pv_node = 1;
                break; /* If the eval is a mate, we can break. */
            }
        }
        if (temp.eval < beta && temp.eval > alpha) {
            is_pv_node = 1;
        }
        i++;
    }
    if (!is_pv_node) {
        return get_best_move_black(the_game, depth, 2*MIN_EVAL, 2*MAX_EVAL, ht);
    }
    _ht_insert_pos(ht, the_game, depth, best, min, PV_NODE);
    create_a_minimax_move_eval(&temp, min, PV_NODE, best);
    return temp;
}



void order_moves(game *g, move *all_moves, double *move_values, char depth, HashTable *ht) {
    /* We need to order the moves so the moves expected to be better will be first: */
    move hash_move = END;
    const void *tempvoid;
    board *the_board = g->current_position;

    tempvoid = _ht_search_pos(ht, g, depth - 1, PV_NODE);
    if (tempvoid != NULL && ((ht_move_eval_struct *)tempvoid)->best_move != END) {
        hash_move = ((ht_move_eval_struct *)tempvoid)->best_move;
    } 
    /* Go through all the moves and assign values: */
    for (int i = 0; all_moves[i] != END; i++) {
        move_values[i] = 0;
        if (all_moves[i] == hash_move) {
            move_values[i] = 10000;
        }
        else {
            double move_eval_change = 0;
            int push_score = push_move_score(the_board, all_moves[i]) * 1;
            double center_score = center_helping_score(the_board, all_moves[i]) * 2;
            double three_in_a_row_score = 0;
            if (depth > 4) {
                three_in_a_row_score = three_in_a_row_helping_score(the_board, all_moves[i]) * 1;
                if (three_in_a_row_score < 0) {
                    three_in_a_row_score = 0;
                }
            }
            move_values[i] = push_score + center_score + three_in_a_row_score;
            if (all_moves[i] == killer_moves[depth][0]) {
                move_values[i] += 2000;
            }
            else if (all_moves[i] == killer_moves[depth][1]) {
                move_values[i] += 1000;
            } 
            move_values[i] += log(get_history_heuristic(all_moves[i]) + 1);
            
        }
    }
}


void order_moves_queiscence(game *g, move *all_moves, double *move_values, HashTable *ht) {
    /* We need to order the moves so the moves expected to be better will be first: */
    move hash_move = END;
    const void *tempvoid;
    board *the_board = g->current_position;

    tempvoid = _ht_search_pos(ht, g, 0, PV_NODE);
    if (tempvoid != NULL && ((ht_move_eval_struct *)tempvoid)->best_move != END) {
        hash_move = ((ht_move_eval_struct *)tempvoid)->best_move;
    } 
    /* Go through all the moves and assign values: */
    for (int i = 0; all_moves[i] != END; i++) {
        move_values[i] = 0;
        if (all_moves[i] == hash_move) {
            move_values[i] = 10000;
        }
        else {
            double move_eval_change = 0;
            int push_score = push_move_score(the_board, all_moves[i]) * 1;
            double center_score = center_helping_score(the_board, all_moves[i]) * 2;
            move_values[i] = push_score + center_score;
            if (is_capture(the_board, all_moves[i])) {
                move_values[i] += 5000; /* Captures are very good. */
            }
            // print all heirostics, for comparison
        }
    }
}

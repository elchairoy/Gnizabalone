#include "../include/minimax.h"

#define MAX_EVAL 100 /* The maximum evaluation possible. */
#define MIN_EVAL -100 /* The minimum evaluation possible. */

#define EVAL_TO_PRUNE 2 /* In what evaluation the minimax will cut the moves branch because it's too bad. */
char NULL_MOVE_REDUCTION = 2; /* The null move reduction. */
char is_nms = 1;

long int number_of_moves = 0; /* The number of positions scaned. */
long int number_of_ht_found = 0; /* The number of positions found in the hash table. */
long int number_of_ht_inserted = 0; /* The number of positions inserted to the hash table. */
long int number_of_null_moves = 0; /* The number of null moves. */

char starting_color;

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

/* This function gets a board (when is white's move) and the depth and evaluates the position using minimax. */
minimax_eval evaluate_minimax_for_white(game *the_game, char depth, double alpha, double beta, HashTable *ht) {
    move all_moves[MAX_POSSIBLE_MOVES];
    move prev_best = END; /* The best move in the position, in depth - 1. */
    int i = 0;
    minimax_eval temp;
    irreversible_move_info temp_inf;
    char is_check = 0;
    board *b = the_game->current_position;
    const void *tempvoid;
    move best;
    double max = MIN_EVAL;
    char is_pv_node = 0;
    number_of_moves++;
    int move_values[MAX_POSSIBLE_MOVES]; /* The values of the moves. */
/*
    board k;
    char fen[100] = "8/3k4/3p4/p2P1p2/P1KP1P2/8/8/8 w - - 8 5";
    fen_to_board(fen, &k);
    if (compare_boards(&k,b) && depth == 3)
        //print_f("The board is the same!\n");
*/
    /* Search for the position in the hash table: */
    //print_board(b);
    if (check_repetition(the_game) || the_game->number_of_moves_in_game >= 300)/* Repitition */ {
        _ht_insert_pos(ht, the_game, depth, END, 0, PV_NODE);
        create_a_minimax_eval(temp, 0, PV_NODE);
        return temp;
    }
    tempvoid = _ht_search_pos(ht, the_game, depth, CUT_NODE);
    if (tempvoid != NULL) { /* Make sure this would also be cutted. */
        if (((ht_move_eval_struct *)tempvoid)->type == PV_NODE) {
            number_of_ht_found++;
            create_a_minimax_eval(temp, ((ht_move_eval_struct *)tempvoid)->eval, PV_NODE);
            return (temp);
        }
        else if (((ht_move_eval_struct *)tempvoid)->type == CUT_NODE && ((ht_move_eval_struct *)tempvoid)->eval < beta) {
            number_of_ht_found++;
            beta = ((ht_move_eval_struct *)tempvoid)->eval;
        }
        if (alpha >= beta) {
            create_a_minimax_eval(temp, beta, CUT_NODE);
            update_killer_moves(((ht_move_eval_struct *)tempvoid)->best_move);
            update_history_heuristic(((ht_move_eval_struct *)tempvoid)->best_move);
            return temp;
        }
    }  

    if (depth == 0) {
        temp.eval = evaluate(the_game->current_position, starting_color) + get_random_up_to_one();
        _ht_insert_pos(ht, the_game, 0, END, temp.eval, PV_NODE);
        return temp;
    }
    get_possible_moves(b,all_moves,0,0,0); /* Gets all the moves possible. */
    if (is_lost(b,WHITE)) {
        _ht_insert_pos(ht, the_game, depth, END, MIN_EVAL, PV_NODE);
        create_a_minimax_eval(temp, MIN_EVAL, PV_NODE);
        return temp;
    }

    /* NULL MOVE SEARCH */
    if (depth >= NULL_MOVE_REDUCTION && is_nms) {
        number_of_null_moves++;
        is_nms = 0;
        b->whose_turn = BLACK;
        temp = evaluate_minimax_for_black(the_game, depth - NULL_MOVE_REDUCTION, alpha, beta, ht);
        b->whose_turn = WHITE;
        is_nms = 1;
        if (temp.eval >= beta) {
            _ht_insert_pos(ht, the_game, depth, END, temp.eval, CUT_NODE);
            create_a_minimax_eval(temp, beta, CUT_NODE);
            return temp;
        }
    }

    order_moves(the_game, all_moves, move_values, depth, ht); /* Orders the moves. */
    while (all_moves[i] != END) {
        selection_sort_for_moves(all_moves, move_values, i); /* Sorts the moves. */
        //print_move(all_moves[i]);
        temp_inf = get_irrev_move_info(b,all_moves[i]);
        commit_a_move_in_game(the_game,all_moves[i]); /* Commits the move. */
        temp = evaluate_minimax_for_black(the_game, depth - 1,alpha, beta, ht);
        unmake_move_in_game(the_game,all_moves[i],temp_inf);
        if (temp.eval >= beta) {
            _ht_insert_pos(ht, the_game, depth, all_moves[i], temp.eval, CUT_NODE);
            create_a_minimax_eval(temp, beta, CUT_NODE);
            update_killer_moves(all_moves[i]);
            update_history_heuristic(all_moves[i]);
            return temp;
        }
        if (temp.eval > alpha) {
            alpha = temp.eval;
            is_pv_node = 1;
        }
        if (temp.eval > max) {
            max = temp.eval;
            best = all_moves[i];
            if (temp.eval >= MAX_EVAL) {
                _ht_insert_pos(ht, the_game, depth, all_moves[i], MAX_EVAL, PV_NODE);
                create_a_minimax_eval(temp, MAX_EVAL, PV_NODE);
                return temp;
            }
        }

        i++;
    }
    if (is_pv_node) {
        _ht_insert_pos(ht, the_game, depth, best, max, PV_NODE);
        create_a_minimax_eval(temp, max, PV_NODE);
    }
    else {
        _ht_insert_pos(ht, the_game, depth, best, max, CUT_NODE);
        create_a_minimax_eval(temp, max, CUT_NODE);
    }
    return temp;
}
/* This function gets a board (when is black's move) and the depth and evaluates the position using minimax. */
minimax_eval evaluate_minimax_for_black(game *the_game, char depth, double alpha, double beta, HashTable *ht) {
    board *b = the_game->current_position;
    move all_moves[MAX_POSSIBLE_MOVES];
    move prev_best = END; /* The best move in the position, in depth - 1. */
    int i = 0;
    minimax_eval temp;
    irreversible_move_info temp_inf;
    char is_check = 0;
    const void *tempvoid;
    move best;
    double min = MAX_EVAL;
    char is_pv_node = 0;
    number_of_moves++;
    int move_values[MAX_POSSIBLE_MOVES]; /* The values of the moves. */
    //print_board(b);
    if (check_repetition(the_game) || the_game->number_of_moves_in_game >= 300) {
        _ht_insert_pos(ht, the_game, depth, END, 0, PV_NODE);
        create_a_minimax_eval(temp, 0, PV_NODE);
        return temp;
    }
    /*board k;
    char fen[100] = "2k5/8/3p4/p2P1p2/P1KP1P2/8/8/8 b - - 7 4";
    fen_to_board(fen, &k);
    if (compare_boards(&k,b) && depth == 4)
        //print_f("The board is the same!\n");*/

    /* Search for the position in the hash table: */
    tempvoid = _ht_search_pos(ht, the_game, depth, ALL_NODE);
    if (tempvoid != NULL) { /* Make sure this would also be cutted. */
        if (((ht_move_eval_struct *)tempvoid)->type == PV_NODE) {
            number_of_ht_found++;
            create_a_minimax_eval(temp, ((ht_move_eval_struct *)tempvoid)->eval, PV_NODE);
            return (temp);
        }
        else if (((ht_move_eval_struct *)tempvoid)->type == ALL_NODE && ((ht_move_eval_struct *)tempvoid)->eval > alpha) {
            number_of_ht_found++;
            alpha = ((ht_move_eval_struct *)tempvoid)->eval;
        }
        if (alpha >= beta) {
            create_a_minimax_eval(temp, alpha, ALL_NODE);
            update_killer_moves(((ht_move_eval_struct *)tempvoid)->best_move);
            update_history_heuristic(((ht_move_eval_struct *)tempvoid)->best_move);
            return temp;
        }
    }  

    if (depth == 0) {
        temp.eval = evaluate(the_game->current_position, starting_color) + get_random_up_to_one();
        _ht_insert_pos(ht, the_game, 0, END, temp.eval, PV_NODE);
        return temp;
    }
    get_possible_moves(b,all_moves,0,0,0); /* Gets all the moves possible. */
    best = all_moves[0];

    if (is_lost(b,BLACK)) {
        _ht_insert_pos(ht, the_game, depth, END, MAX_EVAL, PV_NODE);
        create_a_minimax_eval(temp, MAX_EVAL, PV_NODE);
        return temp;
    }

    /* NULL MOVE SEARCH */
    if (depth >= NULL_MOVE_REDUCTION && is_nms) {
        number_of_null_moves++;
        is_nms = 0;
        b->whose_turn = WHITE;
        temp = evaluate_minimax_for_white(the_game, depth - NULL_MOVE_REDUCTION, alpha, beta, ht);
        b->whose_turn = BLACK;
        is_nms = 1;
        if (temp.eval <= alpha) {
            _ht_insert_pos(ht, the_game, depth, END, temp.eval, ALL_NODE);
            create_a_minimax_eval(temp, alpha, ALL_NODE);
            return temp;
        }
    }
    

    order_moves(the_game, all_moves, move_values, depth, ht); /* Orders the moves. */
    while (all_moves[i] != END) {
        selection_sort_for_moves(all_moves, move_values, i); /* Sorts the moves. */
        //print_move(all_moves[i]);
        temp_inf = get_irrev_move_info(b,all_moves[i]);
        commit_a_move_in_game(the_game,all_moves[i]); /* Commits the move. */
        temp = evaluate_minimax_for_white(the_game, depth - 1,alpha, beta, ht); /* Checks what is the eval after the move. */
        unmake_move_in_game(the_game,all_moves[i],temp_inf);
        if (temp.eval <= alpha) {
            _ht_insert_pos(ht, the_game, depth, all_moves[i], temp.eval, ALL_NODE);
            create_a_minimax_eval(temp, alpha, ALL_NODE);
            update_killer_moves(all_moves[i]);
            update_history_heuristic(all_moves[i]);
            return temp;
        }
        if (temp.eval < beta) {
            beta = temp.eval;
            is_pv_node = 1;
        }
        if (temp.eval < min) {
            min = temp.eval;
            best = all_moves[i];
            if (temp.eval <= MIN_EVAL) {
                _ht_insert_pos(ht, the_game, depth, all_moves[i], MIN_EVAL, PV_NODE);
                create_a_minimax_eval(temp, MIN_EVAL, PV_NODE);
                return temp;
            }                
        }
        i++;
    }
    //print_board(b);
    if (is_pv_node) {
        _ht_insert_pos(ht, the_game, depth, best, min, PV_NODE);
        create_a_minimax_eval(temp, min, PV_NODE);
    }
    else {
        _ht_insert_pos(ht, the_game, depth, best, min, ALL_NODE);
        create_a_minimax_eval(temp, min, ALL_NODE);
    }
    return temp;
}

/* The main function of minimax for white, returns the best move of the position. */
minimax_eval get_best_move_white(game *the_game, char depth, double alpha, double beta, HashTable *ht) {
    board *b = the_game->current_position;
    move all_moves[MAX_POSSIBLE_MOVES]; /* All the moves possible in the position. */
    int i = 0;
    int j;
    double max = MIN_EVAL; /* The maximun eval possible in the position(maximum = best for white). */
    move prev_best = END; /* The best move in the position, in depth - 1. */
    minimax_eval temp;
    move best;
    irreversible_move_info temp_inf;
    const void *tempvoid = 0;

    number_of_moves++;
    int move_values[MAX_POSSIBLE_MOVES]; /* The values of the moves. */
    char pv_move_found = 1;
    //print_board(b);
    get_possible_moves(b,all_moves,0,0,0); /* Gets all the moves possible. */
    best = all_moves[0]; /* The default move . */

    if (is_lost(b,WHITE)) {
        create_a_minimax_move_eval(temp, MIN_EVAL, PV_NODE, END);
        return temp;
    }
    starting_color = b->whose_turn;
    is_nms = 1;
    order_moves(the_game,all_moves,move_values, depth, ht); /* Orders the moves. */
    while (all_moves[i] != END) {
        selection_sort_for_moves(all_moves, move_values, i); /* Sorts the moves. */
        //print_move(all_moves[i]);
        temp_inf = get_irrev_move_info(b,all_moves[i]);
        commit_a_move_in_game(the_game,all_moves[i]); /* Commits the move. */
        temp = evaluate_minimax_for_black(the_game,depth - 1, max, beta, ht); /* Checks what is the eval after the move. */
        unmake_move_in_game(the_game,all_moves[i],temp_inf);
        if (temp.eval < beta)
            pv_move_found = 1;

        if (temp.eval > max) { /* If the eval is better then the max eval: */
            max = temp.eval; /* Changes max to be it. */
            best = all_moves[i];
            if (temp.eval >= MAX_EVAL)
                break; /* If the eval is a mate, we can break. */
        }
        i++;
    }
    //print_board(b);
    if (!pv_move_found) {
        return get_best_move_white(the_game, depth, 2 * MIN_EVAL, 2 * MAX_EVAL, ht);
    }
    _ht_insert_pos(ht, the_game, depth, best, max, PV_NODE);
    create_a_minimax_move_eval(temp, max, PV_NODE, best);
    return temp;
}

/* The main function of minimax for black, returns the best move of the position. */
minimax_eval get_best_move_black(game *the_game,char depth, double alpha, double beta, HashTable *ht) {
    board *b = the_game->current_position;
    move all_moves[MAX_POSSIBLE_MOVES]; /* All the moves possible in the position. */
    int i = 0;
    int j = 0;
    double min = MAX_EVAL; /* The maximun eval possible in the position(maximum = best for white). */
    move best; /* The best move in the position. */
    minimax_eval temp;
    irreversible_move_info temp_inf;
    const void *tempvoid = 0;

    number_of_moves++;
    char pv_move_found = 1;
    int move_values[MAX_POSSIBLE_MOVES]; /* The values of the moves. */

    //print_board(b);    

    get_possible_moves(b,all_moves,0,0,0); /* Gets all the moves possible. */
    best = all_moves[i]; /* The default move . */
    

    if (is_lost(b,BLACK)) {
        create_a_minimax_move_eval(temp, MAX_EVAL, PV_NODE, END);
        return temp;
    }
    starting_color = b->whose_turn;
    is_nms = 1;

    order_moves(the_game, all_moves, move_values, depth, ht);
    while (all_moves[i] != END) {
        /* Make a selection sort in all_moves, using the values of the moves: */
        selection_sort_for_moves(all_moves, move_values, i);
        //print_move(all_moves[i]);
        temp_inf = get_irrev_move_info(b,all_moves[i]);
        commit_a_move_in_game(the_game,all_moves[i]); /* Commits the move. */
        temp = evaluate_minimax_for_white(the_game, depth - 1, alpha, min, ht); /* Checks what is the eval after the move. */
        unmake_move_in_game(the_game,all_moves[i],temp_inf);
        if (temp.eval > alpha)
            pv_move_found = 1;

        if (temp.eval < min) { /* If the eval is better then the max eval: */
            min = temp.eval; /* Changes max to be it. */
            best = all_moves[i];
            if (temp.eval <= MIN_EVAL) {
                break; /* If the eval is a mate, we can break. */
            }
        }
        i++;
    }
    if (!pv_move_found) {
        return get_best_move_black(the_game, depth, 2 * MIN_EVAL, 2 * MAX_EVAL, ht);
    }
    _ht_insert_pos(ht, the_game, depth, best, min, PV_NODE);
    create_a_minimax_move_eval(temp, min, PV_NODE, best);
    return temp;
}

void order_moves(game *g, move *all_moves, int *move_values, char depth, HashTable *ht) {
    /* We need to order the moves so the moves expected to be better will be first: */
    move hash_move = END;
    char temp;
    const void *tempvoid;
    board *the_board = g->current_position;

    tempvoid = _ht_search_pos(ht, g, depth - 1, PV_NODE);
    if (tempvoid != NULL && ((ht_move_eval_struct *)tempvoid)->best_move != END) {
        hash_move = ((ht_move_eval_struct *)tempvoid)->best_move;
    }  

    double eval = 0;
    if (depth > 2)
        eval = evaluate(the_board, starting_color);
    /* Go through all the moves and assign values: */
    for (int i = 0; all_moves[i] != END; i++) {
        move_values[i] = 0;
        if (all_moves[i] == hash_move) {
            move_values[i] = 1000;
        }
        else {
            //print_board(the_board);
            //print_move(all_moves[i]);
            double move_eval_change = 0;
            int push_score = push_move_score(the_board, all_moves[i]) * 3;
            double center_score = center_helping_score(the_board, all_moves[i]) * 4;
            double cohesion_score = cohesion_helping_score(the_board, all_moves[i]) * 2;
            move_values[i] = push_score + center_score + cohesion_score;
            if (depth > 2) {
                irreversible_move_info inf = get_irrev_move_info(the_board, all_moves[i]);
                commit_a_move_in_game(g, all_moves[i]);
                move_eval_change = evaluate(the_board, starting_color) - eval;
                unmake_move_in_game(g, all_moves[i], inf);
            }
            if (all_moves[i] == killer_moves[depth][0]) {
                move_values[i] += 2000;
            }
            else if (all_moves[i] == killer_moves[depth][1]) {
                move_values[i] += 1000;
            } 
            move_values[i] += move_eval_change * 15000;
            move_values[i] += get_history_heuristic(all_moves[i]);
            // print all heirostics, for comparison
        }
    }
}

#include "analyse.h"

/* Multi-PV version that fills full PV lines for white */
int get_best_pvs_white(game *the_game, char depth, double alpha, double beta,
                       HashTable *ht, int num_pvs, pv_line_t *pv_results) {
    board *b = the_game->current_position;
    move all_moves[MAX_POSSIBLE_MOVES];
    double move_values[MAX_POSSIBLE_MOVES];
    int excluded[MAX_POSSIBLE_MOVES] = {0};
    int total_moves, pv_count = 0;

    get_possible_moves(b, all_moves);
    order_moves(the_game, all_moves, move_values, depth, ht);
    total_moves = 0;
    while (all_moves[total_moves] != END) { selection_sort_for_moves(all_moves, move_values, total_moves); total_moves++; }

    for (pv_count = 0; pv_count < num_pvs; pv_count++) {
        double max = alpha;
        move best = END;
        minimax_eval temp;
        int i = 0;
        char is_pv_node = 0;

        while (i < total_moves) {
            if (excluded[i]) { i++; continue; }

            irreversible_move_info temp_inf = get_irrev_move_info(b, all_moves[i]);
            commit_a_move_in_game(the_game, all_moves[i]);
            temp = evaluate_minimax_for_black(the_game, depth - 1, max, beta, ht);
            unmake_move_in_game(the_game, all_moves[i], temp_inf);

            if (temp.type == NO_TIME) return pv_count;

            if (temp.eval > max) {
                max = temp.eval;
                best = all_moves[i];

                if (temp.eval >= MAX_EVAL) { is_pv_node = 1; break; }
            }
            if (temp.eval < beta && temp.eval > alpha) is_pv_node = 1;

            i++;
        }

        if (!is_pv_node) {
            temp = get_best_move_white(the_game, depth, 2*MIN_EVAL, 2*MAX_EVAL, ht);
            best = temp.m;
            max = temp.eval;
        }

        // Save evaluation
        pv_results[pv_count].eval = max;

        // Reconstruct PV from TT
        pv_results[pv_count].length = 0;
        game temp_game;
        board temp_board; 
        int pv_len = 0;

        temp_board = *the_game->current_position;
        temp_game = *the_game; // shallow copy
        temp_game.current_position = &temp_board;
        pv_results[pv_count].moves[pv_len++] = best;
        commit_a_move_in_game(&temp_game, best);

        while (pv_len < depth) {
            move m;
            if (temp_board.whose_turn == WHITE)
                m = get_best_move_white(&temp_game, depth - pv_len, -200, 200, ht).m;
            else
                m = get_best_move_black(&temp_game, depth - pv_len, -200, 200, ht).m;

            if (m == END) break;

            pv_results[pv_count].moves[pv_len++] = m;
            irreversible_move_info inf = get_irrev_move_info(&temp_board, m);
            commit_a_move_in_game(&temp_game, m);
        }
        pv_results[pv_count].length = pv_len;

        // Exclude this best move for next PV
        for (i = 0; i < total_moves; i++) {
            if (all_moves[i] == best) { excluded[i] = 1; }
        }
    }

    return pv_count;
}


/* Multi-PV version that fills full PV lines for black */
int get_best_pvs_black(game *the_game, char depth, double alpha, double beta,
                       HashTable *ht, int num_pvs, pv_line_t *pv_results) {
    board *b = the_game->current_position;
    move all_moves[MAX_POSSIBLE_MOVES];
    double move_values[MAX_POSSIBLE_MOVES];
    int excluded[MAX_POSSIBLE_MOVES] = {0};
    int total_moves, pv_count = 0;

    get_possible_moves(b, all_moves);
    order_moves(the_game, all_moves, move_values, depth, ht);
    total_moves = 0;
    while (all_moves[total_moves] != END) {selection_sort_for_moves(all_moves, move_values, total_moves); total_moves++; }

    for (pv_count = 0; pv_count < num_pvs; pv_count++) {
        double min = beta;
        move best = END;
        minimax_eval temp;
        int i = 0;
        char is_pv_node = 0;

        while (i < total_moves) {
            if (excluded[i]) {i++; continue;}

            irreversible_move_info temp_inf = get_irrev_move_info(b, all_moves[i]);
            commit_a_move_in_game(the_game, all_moves[i]);
            temp = evaluate_minimax_for_white(the_game, depth - 1, alpha, min, ht);
            unmake_move_in_game(the_game, all_moves[i], temp_inf);

            if (temp.type == NO_TIME) return pv_count;

            if (temp.eval < min) {
                min = temp.eval;
                best = all_moves[i];

                if (temp.eval <= MIN_EVAL) { is_pv_node = 1; break; }
            }
            if (temp.eval < beta && temp.eval > alpha) is_pv_node = 1;

            i++;
        }

        if (!is_pv_node) {
            temp = get_best_move_black(the_game, depth, 2*MIN_EVAL, 2*MAX_EVAL, ht);
            best = temp.m;
            min = temp.eval;
        }

        // Save evaluation
        pv_results[pv_count].eval = min;

        // Reconstruct PV from TT
        pv_results[pv_count].length = 0;
        game temp_game;
        board temp_board; 
        int pv_len = 0;

        temp_board = *the_game->current_position;
        temp_game = *the_game; // shallow copy
        temp_game.current_position = &temp_board;
        pv_results[pv_count].moves[pv_len++] = best;
        commit_a_move_in_game(&temp_game, best);

        while (pv_len < depth) {
            move m;
            if (temp_board.whose_turn == WHITE)
                m = get_best_move_white(&temp_game, depth - pv_len, -200, 200, ht).m;
            else
                m = get_best_move_black(&temp_game, depth - pv_len, -200, 200, ht).m;

            if (m == END) break;

            pv_results[pv_count].moves[pv_len++] = m;
            irreversible_move_info inf = get_irrev_move_info(&temp_board, m);
            commit_a_move_in_game(&temp_game, m);
        }
        pv_results[pv_count].length = pv_len;

        // Exclude this best move for next PV
        for (i = 0; i < total_moves; i++) {
            if (all_moves[i] == best) { excluded[i] = 1;}
        }
    }

    return pv_count;
}

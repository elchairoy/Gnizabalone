#include "../include/make_move.h"

void commit_a_move_in_board(board *the_board, move m){
    if (get_move_type(m) == ASIDE) {
        /* First lets get the direction of the ROW (not the move direction). */
        char src_row = get_src_row(m), src_col = get_src_col(m);
        char end_of_line_row = get_end_of_line_row(m), end_of_line_col = get_end_of_line_col(m);
        enum directions line_direction, direction = get_direction(m);
        line_direction = get_direction_between_squares(src_row, src_col, end_of_line_row, end_of_line_col);
        /* Now go over the line and make the move for all the marbles. */
        char marb_row = src_row, marb_col = src_col;
        while (1) {
            change_the_square(the_board, marb_row, marb_col, empty);
            change_the_square_in_direction(the_board, marb_row, marb_col, direction, 1, the_board->whose_turn+1);
            if (marb_row == end_of_line_row && marb_col == end_of_line_col)
                break;
            get_new_cords_in_direction(marb_row, marb_col, line_direction, 1);
        }
    }
    else {
        char src_row = get_src_row(m), src_col = get_src_col(m);
        enum directions direction = get_direction(m);
        char marb_row = src_row, marb_col = src_col;
        char prev = go_to_square(the_board, marb_row, marb_col);
        change_the_square(the_board, marb_row, marb_col, empty);
        while (1) {
            char temp = get_marb_in_direction(the_board, marb_row, marb_col, direction, 1);
            if (temp == -1 || prev == empty)
                break;
            change_the_square_in_direction(the_board, marb_row, marb_col, direction, 1, prev);
            prev = temp;
            get_new_cords_in_direction(marb_row, marb_col, direction, 1);
        }
    }
    the_board->whose_turn = !the_board->whose_turn;
}

void commit_a_move_in_game(game *the_game, move m){
        /* count all the marbles and check if there are 14 */

    commit_a_move_in_board(the_game->current_position, m);
    the_game->moves[the_game->number_of_moves_in_game] = m;
    the_game->number_of_moves_in_game++;
    the_game->moves[the_game->number_of_moves_in_game] = 0;



}

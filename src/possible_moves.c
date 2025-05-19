#include "../include/possible_moves.h"



/* This function gets all the possible forward moves for a square. */
char get_all_forward_moves_for_square(board *the_board, move *all_moves, char src_row, char src_col) {
    /* Let's calculate the forward moves. 
    We look at each direction, and search for one of three patterns (A = ally, E = enemy, 0 = empty, - = out of bounds):
    1. A* 0
    2. A* E+ 0
    3. A* E+ -
    where alway the number of no_A >= no_E, and no_A < MAX_PUSHED_ALLY, E < MAX_PUSHED_ENEMY
    */
    char no_moves = 0;
    for (enum directions d = 0; d < 6; d++) {
        char no_ally = 1;
        char no_enemy = 0;
        char temp = get_marb_in_direction(the_board, src_row, src_col, d, no_ally);
        while (no_ally < MAX_PUSHED_ALLY && temp == the_board->whose_turn + 1) {
            no_ally++;
            temp = get_marb_in_direction(the_board, src_row, src_col, d, no_ally);
        }
        no_ally--;
        if (temp == empty) {
            move m;
            create_a_move(m, src_row, src_col, d, src_row, src_col);
            all_moves[no_moves] = m;
            no_moves++;
            continue;
        }
        while (no_enemy <= no_ally && temp == get_enemy_marble(the_board->whose_turn + 1)) {
            no_enemy++;
            temp = get_marb_in_direction(the_board, src_row, src_col, d, (no_ally + no_enemy));
        }
        if (temp == empty || (no_enemy >= 1 && temp == -1)) {
            move m;
            create_a_move(m, src_row, src_col, d, src_row, src_col);
            all_moves[no_moves] = m;
            no_moves++;
            continue;
        }
    }
    return no_moves;
}

/* This function returns all the strait lines of the current player in the square. */
char get_all_lines_in_square(board *the_board, char src_row, char src_col, char (*all_lines)[4]) {
    /* We get all lines that start in this square, and contains only A and less than MAX_LINE_LENGTH. */
    char no_lines = 0;
    for (enum directions d = 0; d < 6; d++) {
        char row = src_row;
        char col = src_col;
        char no_ally = 1;
        char temp = get_marb_in_direction(the_board, row, col, d, no_ally);
        while (no_ally < MAX_LINE_LENGTH && temp == the_board->whose_turn + 1) {
            all_lines[no_lines][0] = row;
            all_lines[no_lines][1] = col;
            all_lines[no_lines][2] = d;
            all_lines[no_lines][3] = no_ally + 1;
            no_lines++;
            no_ally++;
            temp = get_marb_in_direction(the_board, row, col, d, no_ally);
        }
    }
    return no_lines;
}

/* This function returns all the strait lines of the current player in the board. */
char get_all_lines_in_board(board *the_board, char all_lines[MAX_LINES_IN_BOARD][4]) {
    /* We get all lines that start in this square, and contains only A and less than MAX_LINE_LENGTH. */
    char no_lines = 0;
    for (char src_row = -RADIUS + 1; src_row <= RADIUS - 1; src_row++) {
        for (char src_col = -RADIUS + 1; src_col <= RADIUS - 1; src_col++) {
            char marb = get_marb_in_square(the_board, src_row, src_col);
            if (marb == the_board->whose_turn + 1) {
                no_lines += get_all_lines_in_square(the_board, src_row, src_col, &all_lines[no_lines]);
            }
        }
    }
    all_lines[no_lines][3] = END;
    no_lines = remove_line_duplicates(all_lines, no_lines);

    return no_lines;
}

/* This function returns all the possible moves a line can do. */
char get_all_aside_moves_for_line(board *the_board, move *all_moves, char line[4]) {
    /* We need to check the 4 other directions. 
    The aside move is possible only if all the places the marbs will go to are empty */
    char no_moves = 0;
    char src_row = line[0], src_col = line[1];
    char line_d = line[2];
    char length = line[3];
    for (enum directions move_d = 0; move_d < 6; move_d++) {
        if (line_d == move_d || get_backward_direction(line_d) == move_d) {
            continue;
        }
        char row = src_row;
        char col = src_col;
        char no_ally = 0;
        char is_possible = 1;
        while (1) {
            char temp = get_marb_in_direction(the_board, row, col, move_d, 1);
            if (temp != empty) {
                is_possible = 0;
                break;
            }
            if (no_ally >= length - 1) {    
                break;
            }
            get_new_cords_in_direction(row, col, line_d, 1);
            no_ally++;
        }
        if (is_possible == 1) {
            move m;
            create_a_move(m, src_row, src_col, move_d, row, col);
            all_moves[no_moves] = m;
            no_moves++;
        }
    }
    return no_moves;
}

/* This function gets the move the long way (calculates the moves of everything). */
void get_all_moves_by_calculating_everything(board *the_board, move *all_moves){
    char no_moves = 0;
    /* First phase - calcualte all forward moves: */
    for (char src_row = -RADIUS + 1; src_row <= RADIUS - 1; src_row++) {
        for (char src_col = -RADIUS + 1; src_col <= RADIUS - 1; src_col++) {
            char marb = get_marb_in_square(the_board, src_row, src_col);
            if (marb == the_board->whose_turn + 1) {
                no_moves += get_all_forward_moves_for_square(the_board, all_moves + no_moves, src_row, src_col);
            }
        }         
    }

    /* Second phase - calculate all aside moves: */
    char all_lines[MAX_LINES_IN_BOARD][4];
    char no_lines = get_all_lines_in_board(the_board, all_lines);
    for (char i = 0; i < no_lines; i++) {
        no_moves += get_all_aside_moves_for_line(the_board, all_moves + no_moves, all_lines[i]);
    }

    all_moves[no_moves] = END;
}


/* This function calculates all possible moves in position based on previous position's array of moves. 
    The function just recalculates the moves that were changed meanwhile. */
void get_possible_moves(board *the_board, move *new_all_moves ,move *all_moves, move last_move, irreversible_move_info inf){
    get_all_moves_by_calculating_everything(the_board, new_all_moves);
}




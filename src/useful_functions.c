/*
 Created by meir on 30/11/2021.
*/

#include "../include/useful_functions.h"

char bounds_grid[11][11] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};


char get_direction_between_squares(char src_row, char src_col, char dest_row, char dest_col){
    enum directions line_direction = -1;
    if (get_is_left_right_of(src_row, src_col, dest_row, dest_col)) {
        if (src_col < dest_col)
            line_direction = RIGHT;
        else
            line_direction = LEFT;
    }
    else if (get_is_up_right_down_left_of(src_row, src_col, dest_row, dest_col)) {
        if (src_row < dest_row)
            line_direction = UP_RIGHT;
        else
            line_direction = DOWN_LEFT;
    }
    else if (get_is_up_left_down_right_of(src_row, src_col, dest_row, dest_col)) {
        if (src_row < dest_row)
            line_direction = UP_LEFT;
        else
            line_direction = DOWN_RIGHT;
    }
    return line_direction;
}


char * PIECES1[4] = {    "·",        // empty
    "●", // black marble
    "○", // white marble
    "⬜" // OUT OF BOUNDS
};




char compare_boards(board *board1, board *board2){
    char answer1 = memcmp(board1, board2, sizeof(board)) == 0;
    return answer1;
}


void print_move(move the_move){
    char src_row = get_src_row(the_move), src_col = get_src_col(the_move);
    char end_of_line_row = get_end_of_line_row(the_move), end_of_line_col = get_end_of_line_col(the_move);
    enum directions direction  = get_direction(the_move);  
    char *direction_str;
    switch (direction) {
        case LEFT: direction_str = "LEFT"; break;
        case UP_LEFT: direction_str = "UP_LEFT"; break;
        case UP_RIGHT: direction_str = "UP_RIGHT"; break;
        case RIGHT: direction_str = "RIGHT"; break;
        case DOWN_RIGHT: direction_str = "DOWN_RIGHT"; break;
        case DOWN_LEFT: direction_str = "DOWN_LEFT"; break;
        default: direction_str = "UNKNOWN"; break;
    }
    if (get_move_type(the_move) == ASIDE) {
        printf("Move: (%d,%d)(%d,%d) %s \n", src_row, src_col, end_of_line_row, end_of_line_col, direction_str);
    } else {
        printf("Move: (%d,%d) %s\n", src_row, src_col, direction_str);
    }
    ;}



const char row_labels[] = "ihgfedcba";
const char row_lensgth[] = {5,6,7,8,9,8,7,6,5}; // Length of each row in the hexagonal board

// Converts axial (x, y) to a string like "D4"
const char* cord_to_label(int x, int y) {
    static char label[4]; // Enough for letter + number + null

    char letter = row_labels[x + RADIUS - 1]; // Convert x to letter
    char number = y + RADIUS; // Convert y to number
    if (x<0)
        number += x;
    number = row_lensgth[x + RADIUS - 1] + 1 - number; // Adjust number based on row length
    number += '0'; // Convert to character
    label[0] = letter;
    label[1] = number;
    label[2] = '\0'; // Null-terminate the string
    return label;
}

void label_to_cord(char* label, char cord[2]) {
    for (int k1 = -RADIUS + 1; k1 < RADIUS; k1++){
        for (int k2 = -RADIUS + 1; k2 < RADIUS; k2++){
            if (strcmp(label, cord_to_label(-k1,-k2)) == 0) {
                cord[0] = k1, cord[1] = k2;
                return;
            }
        }
    }
}

void print_board(board *the_board){
    /* Printing the Abalone board */
    for (int i = 0; i < 26; i++){
        printf(" ");
    }
    printf("\n");
    printf("          5 4 3 2 1\n");
    for (int i = 0; i < 2*RADIUS-1; i++){
        /* print ' ' row_lens[i] times: */
        for (int j = 0; j < 13-row_lensgth[i]; j++){
            printf(" ");
        }
        printf("%c ", row_labels[i]);
        for (int j = row_lensgth[i]; j>=1; j--){
            char lable[4];
            lable[0] = row_labels[i];
            lable[1] = j + '0';
            lable[2] = '\0';
            /* Now we want to find the marb that gives us the label */
            char cord[2];
            label_to_cord(lable, cord);
            printf("%s ", PIECES1[go_to_square(the_board, cord[0],cord[1])]);
        }
        for (int j = 0; j < 13-row_lensgth[i]; j++){
            printf(" ");
        }
        printf("\n");
    }
    for (int i = 0; i < 26; i++){
        printf(" ");
    }
    printf("\n");
}


irreversible_move_info get_irrev_move_info(board *b, move m) {
    if (get_move_type(m) == ASIDE)
        return 0;
    char src_row, src_col, direction, no_pushed = 1;
    irreversible_move_info inf;
    src_row = get_src_row(m), src_col = get_src_col(m);
    direction = get_direction(m);
    for (char i = 1; ; i++) {
        char marb = get_marb_dir(b, src_row, src_col, direction, i);
        if (marb == empty || marb == -1)
            break;
        else 
            no_pushed++;
    }
    create_an_irrev_move_info(inf, no_pushed);
    return inf;
}

/* Unmake move for games: */
void unmake_move_in_game(game *the_game, move m, irreversible_move_info inf) {
    unmake_move_in_board(the_game->current_position, m, inf);
    the_game->moves[the_game->number_of_moves_in_game-1] = 0;
    the_game->number_of_moves_in_game--;
}

/* Unmake move for boards: */
void unmake_move_in_board(board *the_board, move m, irreversible_move_info inf) {
    if (get_move_type(m) == ASIDE) {
        /* In this case just go backwards with all the marbs moved */
        char src_row = get_src_row(m), src_col = get_src_col(m);
        char end_of_line_row = get_end_of_line_row(m), end_of_line_col = get_end_of_line_col(m);
        enum directions line_direction, direction = get_direction(m);
        line_direction = get_direction_between_squares(src_row, src_col, end_of_line_row, end_of_line_col);
        /* Now go over the line and make the move for all the marbles. */
        char marb_row = src_row, marb_col = src_col;
        while (1) {
            change_the_square(the_board, marb_row, marb_col, get_enemy_marble(the_board->whose_turn + 1));
            change_the_square_in_direction(the_board, marb_row, marb_col, direction, 1, empty);
            _ht_update_hash(the_board, marb_row, marb_col, empty, get_enemy_marble(the_board->whose_turn + 1), 0);
            char temp_row = marb_row, temp_col = marb_col;
            get_new_cords_in_direction(&temp_row, &temp_col, direction, 1);
            _ht_update_hash(the_board, temp_row, temp_col, get_enemy_marble(the_board->whose_turn + 1), empty, 0);
            if (marb_row == end_of_line_row && marb_col == end_of_line_col)
                break;
            get_new_cords_in_direction(&marb_row, &marb_col, line_direction, 1);
        }
    }
    else {
        /* In this case firstly go no_pushed times in the direction of the move, 
        empty this square (if it is not out of bounds) and then move every marb backwards until we get to src */
        char src_row = get_src_row(m), src_col = get_src_col(m);
        enum directions direction = get_direction(m), backward_direction = get_backward_direction(direction);
        char no_pushed = get_no_pushed(inf);
        char marb_row = src_row, marb_col = src_col;
        get_new_cords_in_direction(&marb_row, &marb_col, direction, no_pushed);
        char prev = empty;
        if (get_in_bounds(marb_row, marb_col) == 0) {
            prev = the_board->whose_turn? white_marble : black_marble;
            get_new_cords_in_direction(&marb_row, &marb_col, backward_direction, 1);
        }
        while (1) {
            char temp = go_to_square(the_board, marb_row, marb_col);
            change_the_square(the_board, marb_row, marb_col, prev);
            _ht_update_hash(the_board, marb_row, marb_col, temp, prev, 0);
            prev = temp;
            if (marb_row == src_row && marb_col == src_col) 
                break;
            get_new_cords_in_direction(&marb_row, &marb_col, backward_direction, 1);
        }

    }
    the_board->whose_turn = !the_board->whose_turn;
    _ht_update_hash(the_board, 0, 0, 0, 0, 1); // update for turn change
}


/* Check if the game is a draw by repetition: */
char check_repetition(game *the_game) {
    board temp = the_game->initial_position;
    int i;
    int number_of_repetitions = 0;
    for (i = 0; i < the_game->number_of_moves_in_game; i++) {
        commit_a_move_in_board(&temp, the_game->moves[i]);
        if (compare_boards(&temp, the_game->current_position) == 1) {
            number_of_repetitions++;
        }
    }
    if (number_of_repetitions >= 3) {
        return 1;
    }
    else
        return 0;
}

/* This function gets a number k, an array of moves and an array of values, the function returns the move with the k-best value (*the first k moves are already sorted by values*): */
void selection_sort_for_moves(move moves[MAX_POSSIBLE_MOVES / 2], double *values, int k) {
    int i = k, j, max, temp;
    move temp_move;
    max = i;
    for (j = i+1; moves[j] != END; j++) {
        if (values[j] > values[max]) {
            max = j;
        }
    }
    temp = values[i];
    values[i] = values[max];
    values[max] = temp;
    temp_move = moves[i];
    moves[i] = moves[max];
    moves[max] = temp_move;
}



char is_lost(board *b, char color) {
    char no_marb = 0;
    for (int i = -RADIUS + 1; i < RADIUS; i++) {
        for (int j = -RADIUS + 1; j < RADIUS; j++) {
            char marb = get_marb_in_square(b, i, j);
            if (marb == color + 1) {
                no_marb++;
            }
        }
    }
    if (no_marb == 8)
        return 1;
    return 0;
}

char push_move_score(board *b, move m) {
    char src_row = get_src_row(m), src_col = get_src_col(m);
    enum directions d = get_direction(m);
    /* Now just check if the line ends with an enemy marble */
    irreversible_move_info inf = get_irrev_move_info(b, m);
    char no_pushed = get_no_pushed(inf);
    char marb_row = src_row, marb_col = src_col;
    get_new_cords_in_direction(&marb_row, &marb_col, d, no_pushed - 1);
    if (get_marb_in_square(b, marb_row, marb_col) == get_enemy_marble(b->whose_turn + 1)) {
        char temp_row = marb_row, temp_col = marb_col;
        get_new_cords_in_direction(&temp_row, &temp_col, d, 1);
        if (!get_in_bounds(temp_row, temp_col)) {
            return no_pushed + 6;
        }
        return no_pushed + 2;
    }
    return no_pushed;
}

double center_helping_score(board *b, move m) {
    char src_row = get_src_row(m), src_col = get_src_col(m);
    char start_src_dist;
    char temp_row1 = src_row, temp_col1 = src_col;
    char temp_row2, temp_col2;
    get_new_cords_in_direction(&temp_row1, &temp_col1, get_direction(m), 1);
    enum directions direction = get_direction(m);
    /* Return sum of center score difference of start of the line and end of it */
    int score = 0;
    if (get_move_type(m) == ASIDE) {
        enum directions line_direction = get_direction_between_squares(src_row, src_col, get_end_of_line_row(m), get_end_of_line_col(m));
        while (1) {
            temp_row2 = temp_row1, temp_col2 = temp_col1;
            get_new_cords_in_direction(&temp_row2, &temp_col2, direction, 1);
            score += RADIUS - hex_distance(temp_row2, temp_col2, 0, 0) - 1 -
                        (RADIUS - hex_distance(temp_row1, temp_col1, 0, 0) - 1);
            char temp = get_marb_in_square(b, temp_row1, temp_col1);
            if (temp == empty || temp == -1)
                break;
            get_new_cords_in_direction(&temp_row1, &temp_col1, line_direction, 1);
        }
    }
    else {
        char marb_row = src_row, marb_col = src_col;
        while (1) {
            char temp = get_marb_dir1(b, marb_row, marb_col, direction);
            if (temp == b->whose_turn + 1)
                score -= RADIUS - hex_distance(marb_row, marb_col, 0, 0) - 1;
            else if (temp == get_enemy_marble(b->whose_turn + 1))
                score += RADIUS - hex_distance(marb_row, marb_col, 0, 0) - 1;
            get_new_cords_in_direction(&marb_row, &marb_col, direction, 1);
            if (temp == b->whose_turn + 1)
                score += RADIUS - hex_distance(marb_row, marb_col, 0, 0) - 1;
            else if (temp == get_enemy_marble(b->whose_turn + 1))
                score -= RADIUS - hex_distance(marb_row, marb_col, 0, 0) - 1;
            if (temp == empty || temp == -1)
                break;
        }
    }
    return score;
}

int count_lines_of_3(board *b, char row, char col, char color) {
    // Count the number of lines of 3 that the square (row, col) is the center of.
    int count = 0;

    if (!get_in_bounds(row, col) || (color != white_marble && color != black_marble)) {
        return 0; // Out of bounds
    }
    if (get_marb_in_square(b, row, col) == color) {
        for (enum directions d = 1; d < 4; d++) {
            char marb1 = get_marb_dir1(b, row, col, d);
            char marb2 = get_marb_dir1(b, row, col, get_backward_direction(d));
            if (marb1 == color && marb2 == color) {
                count += 1;
            }
        }
    }
    else if (get_marb_in_square(b, row, col) == get_enemy_marble(color)) {
        // Check if the square is occupied by the enemy marble
        for (enum directions d = 1; d < 4; d++) {
            char marb1 = get_marb_dir1(b, row, col, d);
            char marb2 = get_marb_dir1(b, row, col, get_backward_direction(d));
            if (marb1 == get_enemy_marble(color) && marb2 == get_enemy_marble(color)) {
                count -= 1;
            }
        }
    }
    return count;
}

int three_in_a_row_helping_score(board *b, move m) {
    // count the number of lines of 3 in a row that the move creates, and the number of lines of 3 in a row that the move destroys for the enemy
    // we keep 2 boards, one before and one after the move, and we count the lines of 3 in each board.
    // need to check the marbels that moved themselves, and their neighbours
    board counted; // in here we save the squares that we already counted
    memset(&counted, 0, sizeof(board)); // Initialize the counted board
    int score = 0;
    board temp_board = *b; // Copy the board to a temporary board
    commit_a_move_in_board(&temp_board, m); // Commit the move to the temporary board
    char color = b->whose_turn + 1; // Get the color of the current player
    if (get_move_type(m) == ASIDE) {
        char src_row = get_src_row(m), src_col = get_src_col(m);
        enum directions direction_of_line = get_direction_between_squares(src_row, src_col, get_end_of_line_row(m), get_end_of_line_col(m));
        enum directions direction = get_direction(m);
        char marb_row = src_row, marb_col = src_col;
        while (1) {
            char temp = get_marb_in_square(b, marb_row, marb_col);
            if (temp == empty || temp == -1)
                break;
            if (counted.grid[marb_row + RADIUS - 1][marb_col + RADIUS - 1] != 1) {
                score -= count_lines_of_3(b, marb_row, marb_col, color);
                score += count_lines_of_3(&temp_board, marb_row, marb_col, color);
                counted.grid[marb_row + RADIUS - 1][marb_col + RADIUS - 1] = 1; // Mark this square as counted
            }
            // now check the neighbours of the marble, not in the direction of the move or the backward direction
            for (enum directions d = 0; d < 6; d++) {
                char neighbour_row = marb_row, neighbour_col = marb_col;
                get_new_cords_in_direction(&neighbour_row, &neighbour_col, d, 1);
                if (get_in_bounds(neighbour_row, neighbour_col) == 0) {
                    continue; // Skip if the neighbour is out of bounds
                }
                if (counted.grid[neighbour_row + RADIUS - 1][neighbour_col + RADIUS - 1] == 1) {
                    continue; // Skip if this square was already counted
                }
                score -= count_lines_of_3(b, neighbour_row, neighbour_col, color);
                score += count_lines_of_3(&temp_board, neighbour_row, neighbour_col, color);
                counted.grid[neighbour_row + RADIUS - 1][neighbour_col + RADIUS - 1] = 1; // Mark this square as counted
            }
            // now count the lines of 3 in the board after the move, but in the square of the destination of the current marble
            char dst_row = marb_row, dst_col = marb_col;
            get_new_cords_in_direction(&dst_row, &dst_col, direction, 1);
            if (counted.grid[dst_row + RADIUS - 1][dst_col + RADIUS - 1] != 1) {
                score += count_lines_of_3(&temp_board, dst_row, dst_col, color);
                score -= count_lines_of_3(b, dst_row, dst_col, color);
                counted.grid[dst_row + RADIUS - 1][dst_col + RADIUS - 1] = 1; // Mark this square as counted
            }
            for (enum directions d = 0; d < 6; d++) {
                char neighbour_row = dst_row, neighbour_col = dst_col;
                get_new_cords_in_direction(&neighbour_row, &neighbour_col, d, 1);
                if (get_in_bounds(neighbour_row, neighbour_col) == 0) {
                    continue; // Skip if the neighbour is out of bounds
                }
                if (counted.grid[neighbour_row + RADIUS - 1][neighbour_col + RADIUS - 1] == 1) {
                    continue; // Skip if this square was already counted
                }
                score += count_lines_of_3(&temp_board, neighbour_row, neighbour_col, color);
                score -= count_lines_of_3(b, neighbour_row, neighbour_col, color);
                counted.grid[neighbour_row + RADIUS - 1][neighbour_col + RADIUS - 1] = 1; // Mark this square as counted
            }
            if (direction_of_line >= 6) {
                fprintf(stderr, "Error: direction_of_line is out of bounds: %d\n", direction_of_line);
                print_board(b);
                print_move(m);
                exit(EXIT_FAILURE);
            }
            if (marb_row == get_end_of_line_row(m) && marb_col == get_end_of_line_col(m)) {
                break; // If we reached the end of the line, stop
            }
            get_new_cords_in_direction(&marb_row, &marb_col, direction_of_line, 1);
        }
    }
    else {
        char src_row = get_src_row(m), src_col = get_src_col(m);
        enum directions direction = get_direction(m), backward_direction = get_backward_direction(direction);
        char marb_row = src_row, marb_col = src_col;
        while (1) {
            char temp = get_marb_in_square(b, marb_row, marb_col);
            if (temp == empty || temp == -1 || get_in_bounds(marb_row, marb_col) == 0)
                break;
            if (counted.grid[marb_row + RADIUS - 1][marb_col + RADIUS - 1] != 1) {
                score -= count_lines_of_3(b, marb_row, marb_col, color);
                score += count_lines_of_3(&temp_board, marb_row, marb_col, color);
                counted.grid[marb_row + RADIUS - 1][marb_col + RADIUS - 1] = 1; // Mark this square as counted
            }
            // now check the neighbours of the marble, not in the direction of the move or the backward direction
            for (enum directions d = 0; d < 6; d++) {
                char neighbour_row = marb_row, neighbour_col = marb_col;
                get_new_cords_in_direction(&neighbour_row, &neighbour_col, d, 1);
                if (get_in_bounds(neighbour_row, neighbour_col) == 0) {
                    continue; // Skip if the neighbour is out of bounds
                }
                if (counted.grid[neighbour_row + RADIUS - 1][neighbour_col + RADIUS - 1] == 1) {
                    continue; // Skip if this square was already counted
                }
                score -= count_lines_of_3(b, neighbour_row, neighbour_col, color);
                score += count_lines_of_3(&temp_board, neighbour_row, neighbour_col, color);
                counted.grid[neighbour_row + RADIUS - 1][neighbour_col + RADIUS - 1] = 1; // Mark this square as counted
            
            }
            // now count the lines of 3 in the board after the move, but in the square of the destination of the current marble
            char dst_row = marb_row, dst_col = marb_col;
            get_new_cords_in_direction(&dst_row, &dst_col, direction, 1);
            if (get_in_bounds(dst_row, dst_col) == 0) {
                break; // If the destination is out of bounds, stop
            }
            if (counted.grid[dst_row + RADIUS - 1][dst_col + RADIUS - 1] != 1) {
                score += count_lines_of_3(&temp_board, dst_row, dst_col, color);
                score -= count_lines_of_3(b, dst_row, dst_col, color);
                counted.grid[dst_row + RADIUS - 1][dst_col + RADIUS - 1] = 1; // Mark this square as counted
            }
            for (enum directions d = 0; d < 6; d++) {
                char neighbour_row = dst_row, neighbour_col = dst_col;
                get_new_cords_in_direction(&neighbour_row, &neighbour_col, d, 1);
                if (get_in_bounds(neighbour_row, neighbour_col) == 0) {
                    continue; // Skip if the neighbour is out of bounds
                }
                if (counted.grid[neighbour_row + RADIUS - 1][neighbour_col + RADIUS - 1] == 1) {
                    continue; // Skip if this square was already counted
                }
                score += count_lines_of_3(&temp_board, neighbour_row, neighbour_col, color);
                score -= count_lines_of_3(b, neighbour_row, neighbour_col, color);
                counted.grid[neighbour_row + RADIUS - 1][neighbour_col + RADIUS - 1] = 1; // Mark this square as counted
            }
            get_new_cords_in_direction(&marb_row, &marb_col, direction, 1);
        }
    }
    return score;
}

static int initializer = 0;
double get_random(double value) {
    return value; // for now, disable randomness
    initializer++;
    // Generate a random number between -1 and 1
    double random = (double)rand() / RAND_MAX;
    // now scale it to be up to 5% of the value
    random = (random * 2 - 1) * value * 0.005; // Scale to -0.05*value to 0.05*value
    return value + random; // Add the random value to the original value
}

char *my_strndup(const char *s, size_t n) {
    size_t len = strnlen(s, n);           // Don't go past n
    char *new_str = (char *)malloc(len + 1);
    if (!new_str) return NULL;
    memcpy(new_str, s, len);
    new_str[len] = '\0';
    return new_str;
}

void get_board_string(board *b, char *str) {
        // This function returns a string representation of the board.
    // The string is in the format like:
    // "WWWWW000BBBWWW000BBB000WWWW000BBBWWW000...", where W is a white marble, B is a black marble, and 0 is an empty square.
    // staring from (0, -4), (1, -4), (2, -4), ..., (-1, 3), (0, 3), (1, 3), (2, 3), ..., (4, 4)
    int index = 0;
    for (int i = -RADIUS + 1; i < RADIUS; i++) {
        for (int j = -RADIUS + 1; j < RADIUS; j++) {
            if (get_in_bounds(i, j)) {
                char c = get_marb_in_square(b, i, j);
                if (c == white_marble) {
                    str[index++] = 'W';
                } else if (c == black_marble) {
                    str[index++] = 'B';
                } else {
                    str[index++] = '0';
                }
            }
        }
    }
    str[index] = '\0'; // Null-terminate the string
}

void print_board_string(board *b) {
    char str[100];
    get_board_string(b, str);
    printf("boardString: %s\n", str);
}
 
char is_capture(board *b, move m) {
    // if, its a push move, we need to go in that direction while there is no empty square, and then check if the current square is an enemy marble and the next one is out of bounds
    // this assumes that the move is a valid move
    if (get_move_type(m) == ASIDE) {
        return 0; // not a capture
    }
    char src_row = get_src_row(m), src_col = get_src_col(m);
    enum directions direction = get_direction(m);
    char marb_row = src_row, marb_col = src_col;
    get_new_cords_in_direction(&marb_row, &marb_col, direction, 1);
    while (get_in_bounds(marb_row, marb_col)) {
        char marb = get_marb_in_square(b, marb_row, marb_col);
        if (marb == empty) {
            return 0; // not a capture
        }
        if (marb == get_enemy_marble(b->whose_turn + 1)) {
            get_new_cords_in_direction(&marb_row, &marb_col, direction, 1);
            if (!get_in_bounds(marb_row, marb_col)) {
                return 1; // capture
            }
        }
        else {
            get_new_cords_in_direction(&marb_row, &marb_col, direction, 1);
        }
    }
    return 0; // not a capture
}


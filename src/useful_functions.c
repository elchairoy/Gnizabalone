/*
 Created by meir on 30/11/2021.
*/

#include "../include/useful_functions.h"



char get_direction_between_squares(char src_row, char src_col, char dest_row, char dest_col){
    enum directions line_direction = -1;
    if get_is_left_right_of(src_row, src_col, dest_row, dest_col) {
        if (src_col < dest_col)
            line_direction = RIGHT;
        else
            line_direction = LEFT;
    }
    else if get_is_up_right_down_left_of(src_row, src_col, dest_row, dest_col) {
        if (src_row < dest_row)
            line_direction = UP_RIGHT;
        else
            line_direction = DOWN_LEFT;
    }
    else if get_is_up_left_down_right_of(src_row, src_col, dest_row, dest_col) {
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


char is_in_array(char *array, char value){
    int i;
    for (i = 0; array[i] != -1; i++) {
        if (array[i] == value)
            return 1;
    }
    return 0;
}

char compare_boards(board *board1, board *board2){
    return memcmp(board1,board2,sizeof(board)) == 0;
}

void copy_boards(board *board1, board *board2){
    memcpy(board1,board2,sizeof(board));
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



const char row_labels[] = "abcdefghi";
const char row_lensgth[] = {5,6,7,8,9,8,7,6,5}; // Length of each row in the hexagonal board

// Converts axial (x, y) to a string like "D4"
const char* coord_to_label(int x, int y) {
    static char label[4]; // Enough for letter + number + null

    char letter = row_labels[x + RADIUS - 1]; // Convert x to letter
    char number = y + RADIUS + '0'; // Convert y to number
    if (x<0)
        number += x;
    label[0] = letter;
    label[1] = number;
    label[2] = '\0'; // Null-terminate the string
    return label;
}

char* label_to_coord(char* label) {
    for (int k1 = -RADIUS + 1; k1 < RADIUS; k1++){
        for (int k2 = -RADIUS + 1; k2 < RADIUS; k2++){
            if (strcmp(label, coord_to_label(-k1,-k2)) == 0)
                return (char[2]){k1, k2};
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
            memcpy(cord, label_to_coord(lable), 2);
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

char *strrev(char *str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}

irreversible_move_info get_irrev_move_info(board *b, move m) {
    if (get_move_type(m) == ASIDE)
        return 0;
    char src_row, src_col, dst_row, dst_col, direction, no_pushed = 1;
    irreversible_move_info inf;
    src_row = get_src_row(m), src_col = get_src_col(m);
    direction = get_direction(m);
    for (char i = 1; ; i++) {
        char marb = get_marb_in_direction(b, src_row, src_col, direction, i);
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
            if (marb_row == end_of_line_row && marb_col == end_of_line_col)
                break;
            get_new_cords_in_direction(marb_row, marb_col, line_direction, 1);
        }
    }
    else {
        /* In this case firstly go no_pushed times in the direction of the move, 
        empty this square (if it is not out of bounds) and then move every marb backwards until we get to src */
        char src_row = get_src_row(m), src_col = get_src_col(m);
        enum directions direction = get_direction(m), backward_direction = get_backward_direction(direction);
        char no_pushed = get_no_pushed(inf);
        char marb_row = src_row, marb_col = src_col;
        get_new_cords_in_direction(marb_row, marb_col, direction, no_pushed);
        char prev = empty;
        if (get_in_bounds(marb_row, marb_col) == 0) {
            prev = the_board->whose_turn? white_marble : black_marble;
            get_new_cords_in_direction(marb_row, marb_col, backward_direction, 1);
        }
        while (1) {
            char temp = go_to_square(the_board, marb_row, marb_col);
            change_the_square(the_board, marb_row, marb_col, prev);
            prev = temp;
            if (marb_row == src_row && marb_col == src_col) 
                break;
            get_new_cords_in_direction(marb_row, marb_col, backward_direction, 1);
        }

    }
    the_board->whose_turn = !the_board->whose_turn;
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
    if (number_of_repetitions >= 2) {
        return 1;
    }
    else
        return 0;
}

/* This function gets a number k, an array of moves and an array of values, the function returns the move with the k-best value (*the first k moves are already sorted by values*): */
void selection_sort_for_moves(move moves[MAX_POSSIBLE_MOVES / 2], int *values, int k) {
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

char is_same_line(char *line1, char *line2) {
    char r = line1[0], c = line1[1];
    get_new_cords_in_direction(r, c, line1[2], line1[3] - 1);
    if (r == line2[0] && c == line2[1] && line1[3] == line2[3]) {
        return 1;
    }
    return 0;
}

int remove_line_duplicates(char (*arr)[4], int n) {
    int write_index = 0;

    for (int i = 0; i<n; ++i) {
        int is_duplicate = 0;
        for (int j = 0; j < write_index; ++j) {
            if (is_same_line(arr[i], arr[j])) {
                is_duplicate = 1;
                break;
            }
        }
        if (!is_duplicate) {
            memcpy(arr[write_index], arr[i], sizeof(arr[i]));
            write_index++;
        }
    }
    arr[write_index][3] = END;
    return write_index;  // new length
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
    get_new_cords_in_direction(marb_row, marb_col, d, no_pushed - 1);
    if (get_marb_in_square(b, marb_row, marb_col) == get_enemy_marble(b->whose_turn + 1)) {
        char temp_row = marb_row, temp_col = marb_col;
        get_new_cords_in_direction(temp_row, temp_col, d, 1);
        if (!get_in_bounds(temp_row, temp_col)) {
            return no_pushed + 4;
        }
        return no_pushed + 2;
    }
    return no_pushed;
}

double center_helping_score(board *b, move m) {
    char src_row = get_src_row(m), src_col = get_src_col(m);
    char start_src_dist = hex_distance(src_row, src_col, 0, 0);
    char temp_row1 = src_row, temp_col1 = src_col;
    char temp_row2, temp_col2;
    get_new_cords_in_direction(temp_row1, temp_col1, get_direction(m), 1);
    enum directions direction = get_direction(m);
    /* Return sum of center score difference of start of the line and end of it */
    int score = 0;
    if (get_move_type(m) == ASIDE) {
        enum directions line_direction = get_direction_between_squares(src_row, src_col, get_end_of_line_row(m), get_end_of_line_col(m));
        while (1) {
            temp_row2 = temp_row1, temp_col2 = temp_col1;
            get_new_cords_in_direction(temp_row2, temp_col2, direction, 1);
            score += RADIUS - hex_distance(temp_row2, temp_col2, 0, 0) - 1 -
                        (RADIUS - hex_distance(temp_row1, temp_col1, 0, 0) - 1);
            char temp = get_marb_in_square(b, temp_row1, temp_col1);
            if (temp == empty || temp == -1)
                break;
            get_new_cords_in_direction(temp_row1, temp_col1, line_direction, 1);
        }
    }
    else {
        char marb_row = src_row, marb_col = src_col;
        while (1) {
            char temp = get_marb_in_direction(b, marb_row, marb_col, direction, 1);
            score -= RADIUS - hex_distance(marb_row, marb_col, 0, 0) - 1;
            get_new_cords_in_direction(marb_row, marb_col, direction, 1);
            score += RADIUS - hex_distance(marb_row, marb_col, 0, 0) - 1;
            if (temp == empty || temp == -1)
                break;
        }
    }
    return score;
}

double cohesion_helping_score(board *b, move m) {
    char src_row = get_src_row(m), src_col = get_src_col(m);
    char start_src_dist = hex_distance(src_row, src_col, 0, 0);
    char temp_row1 = src_row, temp_col1 = src_col;
    char temp_row2, temp_col2;
    get_new_cords_in_direction(temp_row1, temp_col1, get_direction(m), 1);
    char end_src_dist = hex_distance(temp_row1, temp_col1, 0, 0);
    enum directions direction = get_direction(m);
    char start_end_dist, end_end_dist;
    /* Return sum of center score difference of start of the line and end of it */
    int score = 0;
    if (get_move_type(m) == ASIDE) {
        enum directions line_direction = get_direction_between_squares(src_row, src_col, get_end_of_line_row(m), get_end_of_line_col(m));
        while (1) {
            temp_row2 = temp_row1, temp_col2 = temp_col1;
            get_new_cords_in_direction(temp_row2, temp_col2, direction, 1);
            score += get_no_neighbours(b, temp_row2, temp_col2, b->whose_turn) -
                        get_no_neighbours(b, temp_row1, temp_col1, b->whose_turn);
            char temp = get_marb_in_square(b, temp_row1, temp_col1);
            if (temp == empty || temp == -1)
                break;
            get_new_cords_in_direction(temp_row1, temp_col1, line_direction, 1);
        }
    }
    else {
        char marb_row = src_row, marb_col = src_col;
        enum directions direction = get_direction(m), backward_direction = get_backward_direction(direction);
        while (1) {
            char temp = get_marb_in_direction(b, marb_row, marb_col, direction, 1);
            score -= get_no_neighbours(b, marb_row, marb_col, b->whose_turn);
            get_new_cords_in_direction(marb_row, marb_col, direction, 1);
            score += get_no_neighbours(b, marb_row, marb_col, b->whose_turn);
            if (temp == empty || temp == -1)
                break;
        }
    }
    return score;
}

static int initializer = 0;
double get_random_up_to_one() {
    return 0;
    initializer++;
    srand(time(NULL) + initializer);
    // Generate a random number between -1 and 1
    double random = (double)rand() / RAND_MAX;
    // Scale it to the range -1 to 1
    random = random * 2 - 1;
    return random / 100000.0;
}

char *my_strndup(const char *s, size_t n) {
    size_t len = strnlen(s, n);           // Don't go past n
    char *new_str = (char *)malloc(len + 1);
    if (!new_str) return NULL;
    memcpy(new_str, s, len);
    new_str[len] = '\0';
    return new_str;
}

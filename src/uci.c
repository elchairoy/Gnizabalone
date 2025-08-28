#include "../include/uci.h"
#include "evaluation.h"

extern long int number_of_moves;
extern long int number_of_ht_inserted;
extern long int number_of_ht_found;
long long int number_of_moves_test = 0;
extern char evaluation_function_number;

extern int history_heuristic_push[2*RADIUS - 1][2*RADIUS - 1][6];
extern int history_heuristic_aside[2*RADIUS - 1][2*RADIUS - 1][6][2*RADIUS - 1][2*RADIUS - 1];

/* From location (e2) to number (12): */
#define get_square_number(column, row) ((row - '0' - 1) * 8 + (column - 'a'))
/* From number(12) to location (e2): */
#define get_square_loc(square_num) (strcat((char[2]){(char)'a' + (square_num % 8), '\0'}, (char[2]){(char)'1' + (square_num / 8), '\0'}))
#define mirrored_square(i) ((NUMBER_OF_ROWS - get_row(i) - 1)*8 + get_column(i))

int player_move(game *the_game, char *str)
{
    /*ask for a move, validate it then comite the moveand request more info from user if needed*/
    board *the_board = the_game->current_position;
    int i = 0;
    char move_str[6];
    move all_moves[MAX_POSSIBLE_MOVES];
    char src_square, dst_square;
    move temp = -1;

    /* If there is a whitespace - ignore what comes after it. */
    while (str[i] != '\0')
    {
        if (str[i] == ' ' || str[i] == '\n') {
            move_str[i] = '\0';
            break;
        }
        move_str[i] = str[i];
        i++;
    }
    move_str[i] = '\0';
    
    char src_label[3], dst_label[3], is_aside;
    char *src_square_str = my_strndup(move_str, 2);
    char *dst_square_str = my_strndup(move_str + 2, 2);
    char end_of_line_row=100, end_of_line_col=100;
    if (move_str[4] == '\0')
        is_aside = 0;
    else {
        is_aside = 1;
        char *end_of_line_square_str = my_strndup(move_str + 4, 2);
        char end_of_line_cord[2];
        label_to_coord(end_of_line_square_str, end_of_line_cord);
        end_of_line_row = end_of_line_cord[0], end_of_line_col = end_of_line_cord[1];
    }

    char src_cord[2];
    label_to_coord(src_square_str, src_cord);
    char src_row = src_cord[0], src_col = src_cord[1];
    char dst_cord[2];
    label_to_coord(dst_square_str, dst_cord);
    char dst_row = dst_cord[0], dst_col = dst_cord[1];
    char d = get_direction_between_squares(src_row, src_col, dst_row, dst_col);

    get_possible_moves(the_board, all_moves, 0, 0, 0);
    
    if (!get_is_in_line(src_row, src_col, dst_row, dst_col)) {
        // then its a different notation of an aside move - 
        // look for possible moves as follows:
        // look to each one of the sides of the dst (call it c), and check if its possible to have an aside move c -> dst with end of line = src.
        char end_of_line_row = src_row, end_of_line_col = src_col;
        for (enum directions d = 0; d < 6; d++)
        {
            char temp_row = dst_row, temp_col = dst_col;
            get_new_cords_in_direction(&temp_row, &temp_col, d, 1);
            if (get_in_bounds(temp_row, temp_col) && get_marb_in_square(the_board, temp_row, temp_col) == the_board->whose_turn + 1)
            {
                // d -> opposite direction of the move
                create_a_move(temp, temp_row, temp_col, get_backward_direction(d), end_of_line_row, end_of_line_col);
                move temp2;
                create_a_move(temp2, end_of_line_row, end_of_line_col, get_backward_direction(d), temp_row, temp_col);
                for (int i = 0; all_moves[i] != END; i++)
                {
                    if (all_moves[i] == temp || all_moves[i] == temp2) {
                        // then we found a valid move
                        commit_a_move_in_game(the_game, temp);
                        return 1;
                    }
                }
            }
        }
    }
    
    temp = -1;


    for (int i = 0; all_moves[i] != END; i++)
    {
        //print_move(all_moves[i]);
        if (((get_src_row(all_moves[i]) == src_row && get_src_col(all_moves[i]) == src_col)||(get_src_row(all_moves[i]) == end_of_line_row && get_src_col(all_moves[i]) == end_of_line_col)) && get_direction(all_moves[i]) == d) {
            if (get_move_type(all_moves[i]) == ASIDE && is_aside) {
                if (((get_end_of_line_row(all_moves[i]) == end_of_line_row && get_end_of_line_col(all_moves[i]) == end_of_line_col) || (get_end_of_line_row(all_moves[i]) == src_row && get_end_of_line_col(all_moves[i]) == src_col))) {
                    temp = all_moves[i];
                    break;
                }
            }
            else if (get_move_type(all_moves[i]) == STRAIGHT && !is_aside) {
                temp = all_moves[i];
                break;
            }
        }
    }
    if (temp != -1) {
        commit_a_move_in_game(the_game, temp);
        return 1;
    }
    else {
        printf("Invalid move\n");
        return 0;
    }
}


char eval_function_by_color[2] = {5, 4};
char depth_by_color[2] = {6, 6};
extern char NULL_MOVE_REDUCTION;
extern char MIN_NULL_MOVE;

double bot_move(game *the_game, HashTable *ht, char logs)
{
    HashTable h;
    if(ht->size >= 2000000) {
        ht_clear(ht);
    }

    board *the_board = the_game->current_position;
    minimax_eval bot_move;
    bot_move.eval = 0;
    int i = 1;
    long int change_in_no_of_moves = 0, initial_number_of_moves = number_of_moves;
    struct timespec start, end;
    decay_history_heuristic(0);
    if (the_board->whose_turn == WHITE)
    {   
        evaluation_function_number = eval_function_by_color[0];
        move possible_moves[MAX_POSSIBLE_MOVES];
        get_possible_moves(the_board, possible_moves, 0, 0, 0);
        double last_depth_time = 0.0000001;
        while (i <= depth_by_color[0]){
            if (i < 6) {
                NULL_MOVE_REDUCTION = 2;
                MIN_NULL_MOVE = 2;
            }
            else {
                NULL_MOVE_REDUCTION = 3;
                MIN_NULL_MOVE = 2;
            }
            bot_move = get_best_move_white(the_game, i, -200, 200, ht);
            change_in_no_of_moves = number_of_moves - initial_number_of_moves;
            if (logs == 1) {
            printf("best so far: ");
            print_move(bot_move.m);
            printf("number of moves: %ld\n", change_in_no_of_moves);
            printf("depth = %d\n", i);
            }
            decay_history_heuristic(HISTORY_DECAY);
            i++;
            
        }
        //moves_in_depth(i-1, the_board, 0, 0 ,0);
        //printf("number of all possible positions: %lld\n", number_of_moves_test);
        move m = bot_move.m;
        commit_a_move_in_game(the_game, bot_move.m);
    }
    
    else
    {
        evaluation_function_number = eval_function_by_color[1];
        move possible_moves[MAX_POSSIBLE_MOVES];
        double last_depth_time = 0.0000001;
        while (i <= depth_by_color[1]){
            if (i < 6) {
                NULL_MOVE_REDUCTION = 2;
                MIN_NULL_MOVE = 2;
            }
            else {
                NULL_MOVE_REDUCTION = 3;
                MIN_NULL_MOVE = 2;
            }
            bot_move = get_best_move_black(the_game, i, -200, 200, ht);
            change_in_no_of_moves = number_of_moves - initial_number_of_moves;
            if (logs == 1) {
            printf("best so far: ");
            print_move(bot_move.m);
            printf("number of moves: %ld\n", change_in_no_of_moves);
            printf("depth = %d\n", i);
            }
            decay_history_heuristic(HISTORY_DECAY);
            i++;
            
        }
        //moves_in_depth(i-1, the_board, 0, 0 ,0);
        //printf("number of all possible positions: %lld\n", number_of_moves_test);
        move m = bot_move.m;
        //printf("Eval: %lf\n",evaluate_minimax_for_black(the_game, 0, the_game->moves[no_of_moves-1],get_irrev_move_info(the_game->current_position, the_game->moves[no_of_moves-1]),depth,-10000,10000));
        commit_a_move_in_game(the_game, bot_move.m);   
    }
    
    if (logs != -1)  {
        printf("eval: %lf\n", bot_move.eval);
        print_board(the_board);}
    if(ht->size >= 2000000) {
        ht_clear(ht);
    }
    return bot_move.eval;
}

int check_endgame(game *the_game)
{
    board *the_board = the_game->current_position;
    move all_moves[MAX_POSSIBLE_MOVES];
    if (the_board->whose_turn == WHITE)
    {
        get_all_moves_by_calculating_everything(the_board,all_moves);
        if (is_lost(the_board, WHITE)) {
            printf("BLACK WON\n");
            fflush(stdout);
            return 0;
        }
        else if (all_moves[0] == END) {
            printf("STALMATE 0.5-0.5\n");
            fflush(stdout);
            return 0;
        }
        if (check_repetition(the_game) || the_game->number_of_moves_in_game >= 200) {
            printf("REPETITION 0.5-0.5\n");
            fflush(stdout);
            return 0;
        }
    }
    else
    {
        get_all_moves_by_calculating_everything(the_board,all_moves);
        if (is_lost(the_board, BLACK)) {
            printf("WHITE WON\n");
            fflush(stdout);
            return 0;
        }
        else if (all_moves[0] == END) {
            printf("STALMATE 0.5-0.5\n");
            fflush(stdout);
            return 0;
        }
        if (check_repetition(the_game) || the_game->number_of_moves_in_game >= 200) {
            printf("REPETITION 0.5-0.5\n");
            fflush(stdout);
            return 0;
        }
    }
    return 1;
}

char get_marb_diff(board *b) {
    int white_marb = 0, black_marb = 0;
    for (int i = -RADIUS+1; i < RADIUS; i++) {
        for (int j = -RADIUS+1; j < RADIUS; j++) {
            if (get_marb_in_square(b, i, j) == white_marble) {
                white_marb++;
            }
            else if (get_marb_in_square(b, i, j) == black_marble) {
                black_marb++;
            }
        }
    }
    return white_marb - black_marb;
}

#define DRAW 1
double who_won(game *the_game)
{
    board *the_board = the_game->current_position;
    move all_moves[MAX_POSSIBLE_MOVES];
    if (the_board->whose_turn == WHITE)
    {
        get_all_moves_by_calculating_everything(the_board,all_moves);
        if (is_lost(the_board, WHITE)) {
            return -1;
        }
        else if (all_moves[0] == END || check_repetition(the_game) || the_game->number_of_moves_in_game >= 200) {
            char diff = 0;
            return 0;
            for (int i = -RADIUS+1; i < RADIUS; i++) {
                for (int j = -RADIUS+1; j < RADIUS; j++) {
                    if (get_in_bounds(i,j) == 0) {
                        continue; // skip out of bounds squares
                    }
                    if (get_marb_in_square(the_board, i, j) == white_marble) {
                        diff++;
                    }
                    else if (get_marb_in_square(the_board, i, j) == black_marble) {
                        diff--;
                    }
                }
            }
            //return diff / 10.0;
            evaluation_function_number = 6;
            return evaluate(the_board, WHITE) / 4;
        }
    }
    else
    {
        get_all_moves_by_calculating_everything(the_board,all_moves);
        if (is_lost(the_board, BLACK)) {
            return 1;
        }
        else if (all_moves[0] == END || check_repetition(the_game) || the_game->number_of_moves_in_game >= 200) {
            char diff = 0;
            return 0;
            for (int i = -RADIUS+1; i < RADIUS; i++) {
                for (int j = -RADIUS+1; j < RADIUS; j++) {
                    if (get_in_bounds(i,j) == 0) {
                        continue; // skip out of bounds squares
                    }
                    if (get_marb_in_square(the_board, i, j) == white_marble) {
                        diff++;
                    }
                    else if (get_marb_in_square(the_board, i, j) == black_marble) {
                        diff--;
                    }
                }
            }
            //return diff / 10.0;
            evaluation_function_number = 6;
            return evaluate(the_board, BLACK) / 4;
        }
    }
    return 0;
}


int sum_bytes(const void *ptr, size_t size) {
    const unsigned char *bytes = ptr;
    int sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum += bytes[i];
    }
    return sum;
}


void init_empty_board(board *b) {
    b->whose_turn = BLACK;
    for (int i = -RADIUS+1; i < RADIUS; i++) {
        for (int j = -RADIUS+1; j < RADIUS; j++) {
            if (get_in_bounds(i,j)) {
                change_the_square(b, i, j, empty);
            }
            else {
                change_the_square(b, i, j, 3);
            }
        }
    }
}


void board_string_to_board(board *b, char *str) {
    // This function takes a string representation of the board and fills the board with the marbles.
    // The string is in the format like:
    // "WWWWW000BBBWWW000BBB000WWWW000BBBWWW000...", where W is a white marble, B is a black marble, and 0 is an empty square.
    int index = 0;
    char revstr[2 * RADIUS * 2 * RADIUS]; // Enough space for the reversed string
    // reverse the string to match the order of the squares
    while (str[index] != '\0' && str[index] != '\n') {
        revstr[index] = str[strlen(str) - 1 - index];
        index++;
    }
    revstr[index] = '\0'; // Null-terminate the reversed string
    index = 1; // Reset index to use for filling the board
    str = revstr; // Use the reversed string for filling the board
    for (int i = -RADIUS + 1; i < RADIUS; i++) {
        for (int j = -RADIUS + 1; j < RADIUS; j++) {
            if (get_in_bounds(i, j)) {
                char c = str[index++];
                if (c == 'W' || c == 'w') {
                    change_the_square(b, i, j, white_marble);
                } else if (c == 'B' || c == 'b') {
                    change_the_square(b, i, j, black_marble);
                } else {
                    change_the_square(b, i, j, empty);
                }
            }
        }
    }
    // the last character of the string is the color of the turn
    if (str[0] == 'W' || str[0] == 'w') {
        b->whose_turn = WHITE;
    } else if (str[0] == 'B' || str[0] == 'b') {
        b->whose_turn = BLACK;
    } else {
        b->whose_turn = BLACK; // Default to black if not specified
    }
    //print_board(b);
}

void regualr_opening(board *b) {
    b->whose_turn = BLACK;
    change_the_square(b, -3, -1, black_marble);
    change_the_square(b, -4, 0, black_marble);
    change_the_square(b, -3, 0, black_marble);
    change_the_square(b, -2, 0, black_marble);
    change_the_square(b, -4, 1, black_marble);
    change_the_square(b, -3, 1, black_marble);
    change_the_square(b, -2, 1, black_marble);
    change_the_square(b, -4, 2, black_marble);
    change_the_square(b, -3, 2, black_marble);
    change_the_square(b, -2, 2, black_marble);
    change_the_square(b, -4, 3, black_marble);
    change_the_square(b, -3, 3, black_marble);
    change_the_square(b, -4, 4, black_marble);
    change_the_square(b, -3, 4, black_marble);

    change_the_square(b, 3, 1, white_marble);
    change_the_square(b, 4, 0, white_marble);
    change_the_square(b, 3, 0, white_marble);
    change_the_square(b, 2, 0, white_marble);
    change_the_square(b, 4, -1, white_marble);
    change_the_square(b, 3, -1, white_marble);
    change_the_square(b, 2, -1, white_marble);
    change_the_square(b, 4, -2, white_marble);
    change_the_square(b, 3, -2, white_marble);
    change_the_square(b, 2, -2, white_marble);
    change_the_square(b, 4, -3, white_marble);
    change_the_square(b, 3, -3, white_marble);
    change_the_square(b, 4, -4, white_marble);
    change_the_square(b, 3, -4, white_marble);
}

void begian_daisy_opening(board *b) {
    b->whose_turn = BLACK;
    change_the_square(b, 4, 0, black_marble);
    change_the_square(b, 3, 0, black_marble);
    change_the_square(b, 2, 0, black_marble);
    change_the_square(b, 2, 1, black_marble);
    change_the_square(b, 3, 1, black_marble);
    change_the_square(b, 3, -1, black_marble);
    change_the_square(b, 4, -1, black_marble);
    change_the_square(b, -2, 0, black_marble);
    change_the_square(b, -3, 0, black_marble);
    change_the_square(b, -4, 0, black_marble);
    change_the_square(b, -2, -1, black_marble);
    change_the_square(b, -3, -1, black_marble);
    change_the_square(b, -3, 1, black_marble);
    change_the_square(b, -4, 1, black_marble);

    change_the_square(b, 2, -2, white_marble);
    change_the_square(b, 2, -3, white_marble);
    change_the_square(b, 3, -2, white_marble);
    change_the_square(b, 3, -3, white_marble);
    change_the_square(b, 3, -4, white_marble);
    change_the_square(b, 4, -3, white_marble);
    change_the_square(b, 4, -4, white_marble);
    change_the_square(b, -2, 2, white_marble);
    change_the_square(b, -2, 3, white_marble);
    change_the_square(b, -3, 2, white_marble);
    change_the_square(b, -3, 3, white_marble);
    change_the_square(b, -3, 4, white_marble);
    change_the_square(b, -4, 3, white_marble);
    change_the_square(b, -4, 4, white_marble);
}

void debug_opening(board *b) {
    b->whose_turn = BLACK;
    // start with all black marbles in the center, and the white is in the corners
    char *bs = "0000000000000WWW0000WBBBW00WBBB0BW00WBBB00000WWW00000W0000000W";
    board_string_to_board(b, bs);
}

/* Create a game: */
void create_game(game *g, board *initial_position) {
    g->current_position = initial_position;
    /* Clear moves: */
    for (int i = 0; i < 1000; i++)
        g->moves[i] = 0;
    g->number_of_moves_in_game = 0;
    g->result = -1;

    // copy the initial position
    for (int i = -RADIUS+1; i < RADIUS; i++) {
        for (int j = -RADIUS+1; j < RADIUS; j++) {
            g->initial_position.grid[i + RADIUS - 1][j + RADIUS - 1] = initial_position->grid[i + RADIUS - 1][j + RADIUS - 1];
        }
    }
    g->initial_position.whose_turn = initial_position->whose_turn;

}


void write_dataset_to_file(const char *filename, Dataset *ds) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to open file for writing");
        return;
    }
    for (int i = 0; i < ds->num_positions; i++) {
        fprintf(fp, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                ds->positions[i].features[0], ds->positions[i].features[1],
                ds->positions[i].features[2], ds->positions[i].features[3],
                ds->positions[i].features[4], ds->positions[i].features[5],
                ds->positions[i].features[6], ds->positions[i].features[7],
                ds->positions[i].features[8], ds->positions[i].target);
    }
    fclose(fp);
}

void add_dataset_to_file(const char *filename, Dataset *ds) {
    FILE *fp = fopen(filename, "a");
    if (!fp) {
        perror("Failed to open file for writing");
        return;
    }
    for (int i = 0; i < ds->num_positions; i++) {
        fprintf(fp, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                ds->positions[i].features[0], ds->positions[i].features[1],
                ds->positions[i].features[2], ds->positions[i].features[3],
                ds->positions[i].features[4], ds->positions[i].features[5],
                ds->positions[i].features[6], ds->positions[i].features[7],
                ds->positions[i].features[8], ds->positions[i].target);
    }
    fclose(fp);
}

void save_boardstring_to_file(const char *filename, char boardstrings[][64], int num_positions) {
    FILE *fp = fopen(filename, "a");
    if (!fp) {
        perror("Failed to open file for writing");
        return;
    }
    for (int i = 0; i < num_positions; i++) {
        fprintf(fp, "%s\n", boardstrings[i]);
    }
    fclose(fp);
}

double simulate_number_of_games(HashTable *ht, char depth, int number_of_games) {
    /* Simulate a number of games and return the average score */
    game the_game;
    memset(&the_game, 0, sizeof(game));
    board init;
    memset(&init, 0, sizeof(board));
    init_empty_board(&init);
    begian_daisy_opening(&init);
    create_game(&the_game, &init);
    double total_score = 0.0;
    depth_by_color[0] = depth;
    depth_by_color[1] = depth; 
    double score;
    int j; 
    Dataset *ds = malloc(sizeof(Dataset));
    ds->positions = malloc(sizeof(Position) * 10000);
    ds->num_positions = 0;
    double features[9];
    char boardstrings[MAX_POSSIBLE_MOVES][64];
    for (int i = 0; i < number_of_games; i++) {
        char temp = eval_function_by_color[0];
        eval_function_by_color[0] = eval_function_by_color[1];
        eval_function_by_color[1] = temp;
        temp = depth_by_color[0];
        depth_by_color[0] = depth_by_color[1];
        depth_by_color[1] = temp;
        init_empty_board(&init);
        begian_daisy_opening(&init);
        create_game(&the_game, &init);
        for (j = 0; j < MAX_POSSIBLE_MOVES; j++) {
            get_special_features(the_game.current_position, the_game.current_position->whose_turn, features);
            get_board_string(the_game.current_position, boardstrings[j]);
            memcpy(ds->positions[ds->num_positions].features, features, sizeof(ds->positions[ds->num_positions].features));
            ds->num_positions++;
            bot_move(&the_game, ht, -1);
            if (check_endgame(&the_game) == 0) {
                break;
            }
        }
        score = who_won(&the_game);
        total_score += score;
        for (int k = 0; k < ds->num_positions; k++) {
            ds->positions[k].target = score;
        }
    }
    // Swap the evaluation functions back if the number of games is odd
    if (number_of_games % 2 == 1) {
        char temp = eval_function_by_color[0];
        eval_function_by_color[0] = eval_function_by_color[1];
        eval_function_by_color[1] = temp; // Swap back if odd number of games
    }
    add_dataset_to_file("all_games_deapth_5.csv", ds);
    save_boardstring_to_file("all_boardstrings_5.txt", boardstrings, j);
    return total_score / number_of_games;
}


void print_board_string(board *b, char with_color) {
    char str[2 * RADIUS * 2 * RADIUS]; // Enough space for the string representation
    get_board_string(b, str);
    if (!with_color)
        printf("boardString: %s\n", str);
    else {
        // Print the color of the turn
        if (b->whose_turn == WHITE) {
            printf("boardString: %sW\n", str);
        } else if (b->whose_turn == BLACK) {
            printf("boardString: %sB\n", str);
        } else {
            fprintf(stderr, "Invalid board color\n");
            exit(EXIT_FAILURE);
        }
    }
}

extern double self_play_weights1[WEIGHT_COUNT];
extern double self_play_weights2[WEIGHT_COUNT];
extern double self_play_weights3[WEIGHT_COUNT];
extern double self_play_weights4[WEIGHT_COUNT];
extern double self_play_weights5[WEIGHT_COUNT];
extern double self_play_weights6[WEIGHT_COUNT];
int load_weights(const char *filename, double *weights, int expected_count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open weight file");
        return -1;
    }

    int i = 0;
    while (i < expected_count && fscanf(file, "%lf", &weights[i]) == 1) {
        i++;
    }

    fclose(file);

    if (i != expected_count) {
        fprintf(stderr, "Error: expected %d weights, but got %d\n", expected_count, i);
        return -1;
    }

    return 0;
}

void get_evals_from_file_of_features(char *filename) {
    // This function reads a file of features and evaluates them using the eval function with eval_number = 6
    // It also writes the results (evaluate(...)) to a new file called "evals.txt, in rows"
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Error opening file %s\n", filename);
        return;
    }
    FILE *out_fp = fopen("evals.txt", "w");
    if (!out_fp) {
        printf("Error opening output file evals.txt\n");
        fclose(fp);
        return;
    }
    fprintf(out_fp, "f0,f1,f2,f3,f4,f5,f6,f7,f8,eval\n");
    char line[256];
    long int count = 0;
    double features[NUM_FEATURES];
    char color = BLACK; // Default color
    evaluation_function_number = 6; // Set the evaluation function to 6
    long int flag = 0;
    double score;
    while (fgets(line, sizeof(line), fp)) {
        // Parse the line to get the features
        char *token = strtok(line, ",");
        for (int i = 0; i < NUM_FEATURES && token != NULL; i++) {
            features[i] = atof(token);
            token = strtok(NULL, ",");
        }
        // score will be the target column, which is the last column in the file
        score = atof(token);
        // check if its all 0s, then print an enter and reset to color black
        if (features[0] == 0 && features[1] == 0 && features[2] == 0 && features[3] == 0 &&
            features[4] == 0 && features[5] == 0 && features[6] == 0 && features[7] == 0 && features[8] == 0) {
            if (flag != count - 2) { // to clear edge case of 2 consecutive 0s
                color = BLACK; // Reset color to black
                fprintf(out_fp, "\n");
            }
            flag = count; // Update the flag to the current count
        }
        // Evaluate the features
        double eval = evaluate_features(features, color);
        fprintf(out_fp, "%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf\n",
                features[0], features[1], features[2], features[3], features[4],
                features[5], features[6], features[7], features[8], eval);
        fflush(out_fp);
        count++;
        if (count % 1000 == 0) {
            printf("Processed %ld lines\n", count);
        }
        color = !color; // Alternate color for the next evaluation
    }
    fclose(fp);

}

void selfplay(HashTable *ht, int number_of_games, char depth) {
    // This function plays a number of games against itself and saves the features to a file
    Dataset ds;
    ds.num_positions = 0;
    ds.positions = malloc(sizeof(Position) * 10000); // Allocate memory for positions
    for (int i = 0; i < number_of_games; i++) {
        eval_function_by_color[0] = rand() % 3 + 4; // Randomly choose between 4, 5, 6
        eval_function_by_color[1] = rand() % 3 + 4; // Randomly choose between 4, 5, 6
        double score = simulate_number_of_games(ht, depth, 1);
    }
    free(ds.positions); // Free the allocated memory
}

void get_avarage_nodes_per_move(HashTable *ht, char depth) {
    // This function plays a game against itself and prints the average number of nodes per move
    game the_game;
    board init;
    memset(&the_game, 0, sizeof(game));
    memset(&init, 0, sizeof(board));
    init_empty_board(&init);
    begian_daisy_opening(&init);
    create_game(&the_game, &init);
    int move_count = 0;
    depth_by_color[0] = depth;
    depth_by_color[1] = depth;
    while (1) {
        bot_move(&the_game, ht, -1);
        move_count++;
        printf("npm: %lf\n", number_of_moves / (double)move_count);
        if (check_endgame(&the_game) == 0) {
            break;
        }
    }
    printf("Total nodes: %lf\n", number_of_moves / (double)move_count);
}

void get_lines_to_test_on(const char *input_filename, const char *output_filename, int lines_to_copy) {
    // we want to copy a number of lines from input_filename to output_filename
    // BUT - we want those positions to be good for comparing bot strength
    // so we will only copy positions where eval function *6* gives a low score (between -0.01 and 0.01)
    FILE *in_fp = fopen(input_filename, "r");
    if (!in_fp) {
        printf("Error opening input file %s\n", input_filename);
        return;
    }
    FILE *out_fp = fopen(output_filename, "w");
    if (!out_fp) {
        printf("Error opening output file %s\n", output_filename);
        fclose(in_fp);
        return;
    }
    char line[256];
    double features[NUM_FEATURES];
    char color = BLACK; // Default color
    evaluation_function_number = 6; // Set the evaluation function to 6
    int copied_lines = 0;
    while (fgets(line, sizeof(line), in_fp) && copied_lines < lines_to_copy) {
        line[61] = 'B'; 
        // load the line to board, then get the features
        board b;
        board_string_to_board(&b, line);
        get_special_features(&b, b.whose_turn, features);
        print_board(&b);
        // Evaluate the features
        double eval = evaluate_features(features, color);
        // If eval is between -0.01 and 0.01, copy the line to output file
        if (eval > -0.1 && eval < 0.1) {
            fprintf(out_fp, "%s\n", line);
            fflush(out_fp);
            copied_lines++;
        }
    }
    fclose(in_fp);
    fclose(out_fp);
    printf("Copied %d lines to %s\n", copied_lines, output_filename);
}

char who_is_better(HashTable *ht, char eval_function_1, char eval_function_2, char depth, int number_of_games, char *positions_file) {
    // This function plays a number of games between two evaluation functions and returns the better one
    // But the starting positions are taken from a file of positions
    // 2 games are played for each position, one with each color
    eval_function_by_color[0] = eval_function_1;
    eval_function_by_color[1] = eval_function_2;
    game the_game;
    board init;
    double total_score = 0.0;
    memset(&the_game, 0, sizeof(game));
    FILE *fp = fopen(positions_file, "r");
    if (!fp) {
        printf("Error opening positions file %s\n", positions_file);
        return -1;
    }
    for (int i = 0; i < number_of_games; i++) {
        memset(&init, 0, sizeof(board));
        char line[256];
        if (!fgets(line, sizeof(line), fp)) {
            printf("Not enough lines in positions file %s\n", positions_file);
            break;
        }
        line[61] = 'B'; // Ensure the color to move is black
        line[62] = '\0'; // Ensure null termination
        board_string_to_board(&init, line);
        print_board(&init);
        create_game(&the_game, &init);
        depth_by_color[0] = depth;
        depth_by_color[1] = depth;
        eval_function_by_color[0] = eval_function_1;
        eval_function_by_color[1] = eval_function_2;
        while (1) {
            bot_move(&the_game, ht, -1);
            if (check_endgame(&the_game) == 0) {
                break;
            }
        }
        double score1 = who_won(&the_game);
        printf("Game1 %d: score = %lf\n", i+1, score1);
        // Swap eval functions and play again
        char temp = eval_function_by_color[0];
        eval_function_by_color[0] = eval_function_by_color[1];
        eval_function_by_color[1] = temp;
        board_string_to_board(&init, line);
        print_board(&init);
        create_game(&the_game, &init);
        while (1) {
            bot_move(&the_game, ht, -1);
            if (check_endgame(&the_game) == 0) {
                break;
            }
        }
        double score2 = who_won(&the_game);
        printf("Game2 %d: score = %lf\n", i+1, score2);
        total_score += score1 - score2;
    }
    printf("total score: %lf\n", total_score);
    return total_score;
}

extern double contributions[NUM_FEATURES];
int main()
{
    
	setbuf (stdin, NULL);
	setbuf (stdout, NULL);

	printf ("\nid name Gnizabalon\n");
	printf ("id author Elchai\n");
	fflush (stdout);

    HashTable ht;
    game *the_game = calloc(1, sizeof(*the_game));
    char is_game_on = 0;
    
    /* Initialize ht: */ 
    ht_setup(&ht,sizeof(ht_board_struct),sizeof(ht_move_eval_struct),2000000);

    load_weights("weights_A.txt", self_play_weights1, WEIGHT_COUNT);
    load_weights("weights_B.txt", self_play_weights2, WEIGHT_COUNT);
    eval_function_by_color[0] = 4;
    eval_function_by_color[1] = 5;
    srand(time(NULL)); // Seed the random number generator
    //get_lines_to_test_on("shuffled_boardstrings.txt", "test_positions.txt", 100);
    //who_is_better(&ht, 4, 5, 4, 3, "test_positions.txt");
    //main_fine_tune(&ht);
    //get_avarage_nodes_per_move(&ht, 3);
    //selfplay(&ht, 10000, 5);
    //exit(0);
    //get_evals_from_file_of_features("all_games_deapth_5.csv");
    
    //simulate a game with depth 4: 
    /*
    depth_by_color[0] = 6;
    depth_by_color[1] = 6;

    board init;
    memset(&init, 0, sizeof(board));
    init_empty_board(&init);
    begian_daisy_opening(&init);
    create_game(the_game, &init);
    while (check_endgame(the_game)) {
        bot_move(the_game, &ht, 0);
        print_board_string(the_game->current_position, 0);
    }   */
     
	while(1) {
        is_game_on = uci_parse(the_game, is_game_on, &ht);
    }
    //stop_python_server();
    
	return 0;
}


char line[10000];
/* This function parses the commands. */
char uci_parse(game *the_game, char is_game_on, HashTable *ht)
{       
    board *init = malloc(sizeof(board));
    memset(init, 0, sizeof(board));
    fgets (line, 8192, stdin);  
    
    if (!strncmp (line, "ucinewgame", 10)){
        //eval_function_by_color[0] = 4;
        //eval_function_by_color[1] = 4;
        // the next 2 chars are the depths
        char depth1 = line[11];
        char depth2 = line[13]; 
        depth_by_color[0] = depth1 - '0';
        depth_by_color[1] = depth2 - '0';
        ht_clear(ht);
        init_empty_board(init);
        if (line[15] == 'B')
            begian_daisy_opening(init);
        else if (line[15] == 'R')
            regualr_opening(init);
        else if (line[15] == 'D')
            debug_opening(init);
        else {
            printf("Unknown opening: %s\n", line + 15);
            begian_daisy_opening(init); // Default to daisy opening
        }
        create_game(the_game,init);
        is_game_on = 1;
        print_board_string(the_game->current_position, 0);
    }

    if (!strncmp (line, "position", 8))
	{
		char *posline = line + 9;

		if (!strncmp (posline, "startpos", 8))
		{
            init_empty_board(init);
            begian_daisy_opening(init);
            create_game(the_game,init);
            is_game_on = 1;
		}


		/* need to make some moves on this position as well? */
		posline = strstr (line, "moves");

		if (posline)
		{
			posline += 6;

			while (1)
			{

                player_move(the_game,posline);

                char board_string[62];
                get_board_string(the_game->current_position, (char *)board_string);
                printf("boardString: %s\n", board_string);

				posline = strstr (posline, " ");
            
                if (!posline)
					break;

				posline ++;

                //print_board(the_game->current_position);
			}
            printf("finished moves\n");
		}
        if (!check_endgame(the_game)) {
            is_game_on = 0;
        }
	}
    if (!strncmp (line, "go", 2))
	{
        if (is_game_on) {
            //print_board(the_game->current_position);
            if (line[2] != '\0' && line[2] != '\n')
                depth_by_color[!(the_game->current_position->whose_turn)] = line[3] - '0';
            bot_move(the_game, ht,1);
            print_board_string(the_game->current_position, 0);
            //print_board(the_game->current_position);
            if (!check_endgame(the_game)) {
                is_game_on = 0;
            }
        }
        else {
            printf("Game not started. (use ucinewgame to start a new game)\n");
        }
	}
    if (!strncmp (line, "simulate", 7))
    {
        if (is_game_on) {
            while (1) {
                bot_move(the_game, ht,0);
                print_board_string(the_game->current_position, 1);
                printf("move number: %d\n", the_game->number_of_moves_in_game);
                if (!check_endgame(the_game)) {
                    is_game_on = 0;
                    break;
                }
            }
            
        }
        else {
            printf("Game not started. (use ucinewgame to start a new game)\n");
        }
    }
    if (!strncmp (line, "st", 2))
    {
        double score = 0;
        for (int i = 0; i < 10; i++) {
            ht_clear(ht);
            init_empty_board(init);
            begian_daisy_opening(init);
            create_game(the_game,init);
            char temp = eval_function_by_color[0];
            eval_function_by_color[0] = eval_function_by_color[1];
            eval_function_by_color[1] = temp;
            temp = depth_by_color[0];
            depth_by_color[0] = depth_by_color[1];
            depth_by_color[1] = temp;
            is_game_on = 1;
            if (is_game_on) {
                while (1) {
                    bot_move(the_game, ht,-1);
                    if (!check_endgame(the_game)) {
                        is_game_on = 0;
                        if (i % 2 == 1) 
                            score += who_won(the_game);
                        else
                            score -= who_won(the_game);
                        break;
                    }
                }
                
            }
        }
        printf("score: %lf\n", score);
    }
    if (!strncmp (line, "move", 4))
    {
        char *posline = line + 5;
        player_move(the_game,posline);
        if (!check_endgame(the_game)) {
            is_game_on = 0;
        }
        print_board(the_game->current_position);
        print_board_string(the_game->current_position, 0);
    }
    if (!strncmp (line, "mg", 2))
    {
        char *posline = line + 3;
        if (posline[0] >= 'a' && posline[0] <= 'i' && posline[2] >= 'a' && posline[2] <= 'i' && posline[1] >= '1' && posline[1] <= '9' && posline[3] >= '1' && posline[3] <= '9'){
            if (player_move(the_game,posline) == 0)
                return is_game_on;
            if (!check_endgame(the_game)) {
                is_game_on = 0;
            }
            bot_move(the_game, ht,0);
            if (!check_endgame(the_game)) {
                is_game_on = 0;
            }
        }
        else printf("error\n");
    }
    if (!strncmp (line, "print", 5))
    {
        if (is_game_on) {
            print_board(the_game->current_position);
        }
        else {
            printf("Game not started. (use ucinewgame to start a new game)\n");
        }
    }
    if (!strncmp (line, "fine tune", 9))
    {
        main_fine_tune(ht);
    }
    if (!strncmp (line, "quit", 4)) {
        exit(0);
    }

    return is_game_on;
}



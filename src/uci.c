#include "../include/uci.h"
#include "evaluation.h"

extern long int number_of_moves;
extern long int number_of_ht_inserted;
extern char evaluation_function_number;

extern int history_heuristic_push[2*RADIUS - 1][2*RADIUS - 1][6];
extern int history_heuristic_aside[2*RADIUS - 1][2*RADIUS - 1][6][2*RADIUS - 1][2*RADIUS - 1];

extern double self_play_weights1[WEIGHT_COUNT];
extern double self_play_weights2[WEIGHT_COUNT];
extern double self_play_weights3[WEIGHT_COUNT];


/* From location (e2) to number (12): */
#define get_square_number(column, row) ((row - '0' - 1) * 8 + (column - 'a'))
/* From number(12) to location (e2): */
#define get_square_loc(square_num) (strcat((char[2]){(char)'a' + (square_num % 8), '\0'}, (char[2]){(char)'1' + (square_num / 8), '\0'}))
#define mirrored_square(i) ((NUMBER_OF_ROWS - get_row(i) - 1)*8 + get_column(i))

int player_move(game *the_game, const char *str)
{
    /*ask for a move, validate it then comite the moveand request more info from user if needed*/
    board *the_board = the_game->current_position;
    int i = 0;
    char move_str[6];
    move all_moves[MAX_POSSIBLE_MOVES];
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
    
    char is_aside;
    char *src_square_str = my_strndup(move_str, 2);
    char *dst_square_str = my_strndup(move_str + 2, 2);
    char end_of_line_row=100, end_of_line_col=100;
    if (move_str[4] == '\0')
        is_aside = 0;
    else {
        is_aside = 1;
        char *end_of_line_square_str = my_strndup(move_str + 4, 2);
        char end_of_line_cord[2];
        label_to_cord(end_of_line_square_str, end_of_line_cord);
        end_of_line_row = end_of_line_cord[0], end_of_line_col = end_of_line_cord[1];
    }

    char src_cord[2];
    label_to_cord(src_square_str, src_cord);
    char src_row = src_cord[0], src_col = src_cord[1];
    char dst_cord[2];
    label_to_cord(dst_square_str, dst_cord);
    char dst_row = dst_cord[0], dst_col = dst_cord[1];
    char d = get_direction_between_squares(src_row, src_col, dst_row, dst_col);

    get_possible_moves(the_board, all_moves);
    
    if (!get_is_in_line(src_row, src_col, dst_row, dst_col)) {
        // then its a different notation of an aside move - 
        // look for possible moves as follows:
        // look to each one of the sides of the dst (call it c), and check if its possible to have an aside move c -> dst with end of line = src.
        end_of_line_row = src_row, end_of_line_col = src_col;
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

extern double max_duration;

void print_pv(game *the_game, char depth, HashTable *ht) {
    // This uses the TT to collect the princple variation. 
    if (depth == 0)
        return;
    char color = the_game->current_position->whose_turn;
    move m;
    irreversible_move_info inf;
    print_board(the_game->current_position);
    max_duration = -1;
    if (color == WHITE)
        m = get_best_move_white(the_game, depth, -200,
                                        200, ht).m;
    else
        m = get_best_move_black(the_game, depth, -200,
                                        200, ht).m;
    printf(" ");
    print_move(m);
    inf = get_irrev_move_info(the_game->current_position, m);
    commit_a_move_in_game(the_game, m);
    print_pv(the_game, depth - 1, ht);
    unmake_move_in_game(the_game, m, inf);
}

static char is_decending;
int cmp_pv_eval(const void *a, const void *b) {
    pv_line_t *pa = (pv_line_t *)a;
    pv_line_t *pb = (pv_line_t *)b;
    if (is_decending > 0)
        return (pb->eval > pa->eval) - (pb->eval < pa->eval); // descending
    else
        return (pa->eval > pb->eval) - (pa->eval < pb->eval); // ascending
}

extern char NULL_MOVE_REDUCTION;
extern char MIN_NULL_MOVE;


int print_analysis(game *the_game, char depth, char k, HashTable *ht)
{
    if(ht->size >= HT_CAPACITY) { ht_clear(ht); printf("Hash table cleared\n"); }
    board *the_board = the_game->current_position;
    pv_line_t pv_results[MAX_POSSIBLE_MOVES];  // store full PVs
    int actual_pvs = 0;

    evaluation_function_number = 2;
    if (depth < 6) { NULL_MOVE_REDUCTION = 2; MIN_NULL_MOVE = 2; }
    else { NULL_MOVE_REDUCTION = 3; MIN_NULL_MOVE = 2; }

    max_duration = -1;
    if (the_board->whose_turn == WHITE) {
        actual_pvs = get_best_pvs_white(the_game, depth, -200, 200, ht, k, pv_results);
    } else {
        actual_pvs = get_best_pvs_black(the_game, depth, -200, 200, ht, k, pv_results);
    }

    // Optional: sort PVs by eval for consistent output
    if (the_board->whose_turn == WHITE)
        is_decending = 1;
    else 
        is_decending = -1;

    qsort(pv_results, actual_pvs, sizeof(pv_line_t), cmp_pv_eval);

    
    for (int i = 0; i < actual_pvs; i++) {
        printf("%d) ", i + 1);
        if (pv_results[i].length > 0)
            print_move(pv_results[i].moves[0]);  // root move

        printf("\tPV: ");
        for (int j = 0; j < pv_results[i].length; j++) {
            print_move(pv_results[i].moves[j]);
            printf(" ");
        }
        printf("\t\teval: %.3f\n", pv_results[i].eval);
    }

    // Decide based on evaluation difference
    char color = the_board->whose_turn * 2 - 1;
    if (pv_results[0].eval * color > 0.1 && pv_results[1].eval * color < 0)
        return 1;

    return 0;
}


double get_max_duration(game *the_game) {
    /* After this amount of time he would stop the search and return the move. */
    double remaining_time = the_game->tc.time_left[the_game->current_position->whose_turn];
    double increment = the_game->tc.increment[the_game->current_position->whose_turn];
    if (increment == -1) {
        return -1; // no limit
    }
    return remaining_time / 20.0 + increment / 1.1;
}

double get_soft_bound(game *the_game) {
    /* This is the soft bound for the search - after each depth if the time exceeded this limit, he wouldn't continue to the next depth. */
    double remaining_time = the_game->tc.time_left[the_game->current_position->whose_turn];
    double increment = the_game->tc.increment[the_game->current_position->whose_turn];
    if (increment == -1) {
        return -1; // no limit
    }
    return remaining_time / 40.0 + increment / 2;
}

char eval_function_by_color[2];
char depth_by_color[2] = {6, 6};
extern clock_t start_time;
double bot_move(game *the_game, HashTable *ht, char logs)
{
    board *the_board = the_game->current_position;
    move all_moves[MAX_POSSIBLE_MOVES];
    get_possible_moves(the_board, all_moves);

    minimax_eval best_move, old_best_move;
    old_best_move.m = all_moves[0];
    best_move.eval = 0;

    int color = the_board->whose_turn;
    int max_depth = depth_by_color[color];
    int i = 1;
    long int initial_moves = number_of_moves;
    double last_eval = 0.0;
    double current_depth_time = 0.0;
    double total_time = 0.0;
    double soft_duration_bound;
    max_duration = get_max_duration(the_game);
    soft_duration_bound = get_soft_bound(the_game);

    decay_history_heuristic(0);

    start_time = clock();
    while (i <= max_depth) {
        // Adjust null-move reduction based on depth
        if (i < 6) { NULL_MOVE_REDUCTION = 2; MIN_NULL_MOVE = 2; }
        else { NULL_MOVE_REDUCTION = 3; MIN_NULL_MOVE = 2; }

        evaluation_function_number = eval_function_by_color[color];

        // Select the move depending on color
        if (color == WHITE)
            best_move = get_best_move_white(the_game, i, last_eval - ASPIRATION_WINDOW,
                                           last_eval + ASPIRATION_WINDOW, ht);
        else
            best_move = get_best_move_black(the_game, i, last_eval - ASPIRATION_WINDOW,
                                           last_eval + ASPIRATION_WINDOW, ht);

        last_eval = best_move.eval;

        long int moves_this_depth = number_of_moves - initial_moves;

        current_depth_time = (double)(clock() - start_time) / CLOCKS_PER_SEC;

        if (best_move.type == NO_TIME) {
            best_move = old_best_move;
            if (logs == 1)
                printf("No time left, stopping search (time %.3f sec)\n", current_depth_time);
            break;
        }
        old_best_move = best_move;

        if (logs == 1) {
            printf("best so far: "); print_move(best_move.m); printf("\n");
            printf("depth = %d, moves this depth = %ld\n", i, moves_this_depth);
            printf("time in this move = %.3f sec\n",
                   current_depth_time);
        }

        if (soft_duration_bound != -1 && current_depth_time > soft_duration_bound) {
            if (logs == 1)
                printf("Soft time limit exceeded, stopping search (time %.3f sec)\n", current_depth_time);
            break;
        }

        decay_history_heuristic(HISTORY_DECAY);
        i++;
    }
    total_time = (double)(clock() - start_time) / CLOCKS_PER_SEC;
    the_game->tc.time_left[color] -= total_time;

    if (logs > 0) {
        printf("princple variation:");
        print_pv(the_game, i - 1, ht);
        printf("\n");
    }

    commit_a_move_in_game(the_game, best_move.m);

    // Optional logging and hash table maintenance
    if (logs != -1) { 
        printf("eval: %lf\n", best_move.eval);
        print_board(the_board);
    }
    if(ht->size >= HT_CAPACITY) { ht_clear(ht); printf("Hash table cleared\n"); }

    // Restore increment
    the_game->tc.time_left[color] += the_game->tc.increment[color];
    return best_move.eval;
}

int check_endgame(game *the_game)
{
    board *the_board = the_game->current_position;
    move all_moves[MAX_POSSIBLE_MOVES];
    if (the_board->whose_turn == WHITE)
    {
        get_possible_moves(the_board,all_moves);
        if (is_lost(the_board, WHITE)) {
            printf("BLACK WON\n");
            fflush(stdout);
            return 0;
        }
        if (the_game->tc.time_left[1] <= 0 && the_game->tc.increment[1] != -1) {
            printf("WHITE WON ON TIMEOUT\n");
            fflush(stdout);
            return 0;
        }
        if (all_moves[0] == END) {
            printf("STALMATE 0.5-0.5\n");
            fflush(stdout);
            return 0;
        }
        if (check_repetition(the_game, 0) || the_game->number_of_moves_in_game >= 250) {
            printf("REPETITION 0.5-0.5\n");
            fflush(stdout);
            return 0;
        }
    }
    else
    {
        get_possible_moves(the_board,all_moves);
        if (is_lost(the_board, BLACK)) {
            printf("WHITE WON\n");
            fflush(stdout);
            return 0;
        }
        if (the_game->tc.time_left[0] <= 0 && the_game->tc.increment[0] != -1) {
            printf("BLACK WON ON TIMEOUT\n");
            fflush(stdout);
            return 0;
        }
        if (all_moves[0] == END) {
            printf("STALMATE 0.5-0.5\n");
            fflush(stdout);
            return 0;
        }
        if (check_repetition(the_game, 0) || the_game->number_of_moves_in_game >= 250) {
            printf("REPETITION 0.5-0.5\n");
            fflush(stdout);
            return 0;
        }
    }
    return 1;
}

#define DRAW 1
double who_won(game *the_game)
{
    board *the_board = the_game->current_position;
    move all_moves[MAX_POSSIBLE_MOVES];
    if (the_board->whose_turn == WHITE)
    {
        get_possible_moves(the_board,all_moves);
        if (is_lost(the_board, WHITE)) {
            return -1;
        }
        else if (the_game->tc.time_left[1] <= 0 && the_game->tc.increment[1] != -1) {
            return -1;
        }
        else if (all_moves[0] == END || check_repetition(the_game, 0) || the_game->number_of_moves_in_game >= 250) {
            return 0;
        }
    }
    else
    {
        get_possible_moves(the_board,all_moves);
        if (is_lost(the_board, BLACK)) {
            return 1;
        }
        else if (the_game->tc.time_left[0] <= 0 && the_game->tc.increment[0] != -1) {
            return 1;
        }
        else if (all_moves[0] == END || check_repetition(the_game, 0) || the_game->number_of_moves_in_game >= 250) {
            return 0;
        }
    }
    return 0;
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
    b->hash = _ht_default_hash(b);
}


void board_string_to_board(board *b, char *str) {
    // This function takes a string representation of the board and fills the board with the marbles.
    // The string is in the format like:
    // "WWWWW000BBBWWW000BBB000WWWW000BBBWWW000...", where W is a white marble, B is a black marble, and 0 is an empty square.
    int index = 0;
    memset(b, 0, sizeof(board)); // Clear the board
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
    b->hash = _ht_default_hash(b);
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
    change_the_square(b, -4, 4, black_marble);
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
    change_the_square(b, 4, -3, white_marble);
    change_the_square(b, 3, -2, white_marble);
    change_the_square(b, 2, -2, white_marble);
    change_the_square(b, 4, -3, white_marble);
    change_the_square(b, 3, -3, white_marble);
    change_the_square(b, 4, -4, white_marble);
    change_the_square(b, 3, -4, white_marble);

    b->hash = _ht_default_hash(b);
}

void belgian_daisy_opening(board *b) {
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
    change_the_square(b, -2, 4, white_marble);

    b->hash = _ht_default_hash(b);
}

void debug_opening(board *b) {
    b->whose_turn = BLACK;
    // start with all black marbles in the center, and the white is in the corners
    char *bs = "0W00000BWW000WBB0000WWBW000000B000000BBWW00000BB00000W0000000B";
    board_string_to_board(b, bs);

    b->hash = _ht_default_hash(b);
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
    g->initial_position.hash = initial_position->hash;

    /* Default time control: */
    g->tc.time_left[0] = 10;
    g->tc.time_left[1] = 100;
    g->tc.increment[0] = 0;
    g->tc.increment[1] = -1;
}

int load_weights_set(const char *filename, double *weights, int expected_count, int set_index) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open weight file");
        return -1;
    }

    char line[1024];
    int current_set = 0;
    int i = 0;
    int reading_set = 0;

    while (fgets(line, sizeof(line), file)) {
        // Skip lines that are only whitespace
        int only_space = 1;
        for (char *p = line; *p; p++) {
            if (!isspace(*p)) {
                only_space = 0;
                break;
            }
        }

        if (only_space) {
            // Empty line indicates new set
            if (reading_set) {
                break; // Finished reading desired set
            } else {
                current_set++;
                continue;
            }
        }

        if (current_set == set_index) {
            reading_set = 1;
            if (i >= expected_count) {
                fprintf(stderr, "Error: too many weights in set %d\n", set_index);
                fclose(file);
                return -1;
            }
            if (sscanf(line, "%lf", &weights[i]) != 1) {
                fprintf(stderr, "Error: invalid number in set %d\n", set_index);
                fclose(file);
                return -1;
            }
            i++;
        }
    }

    fclose(file);

    if (i != expected_count) {
        fprintf(stderr, "Error: expected %d weights in set %d, but got %d\n", expected_count, set_index, i);
        return -1;
    }

    return 0;
}



typedef struct {
    double features[12]; // Assuming 9 features
    double target;      // The game result from this position
} Position;
typedef struct {
    Position *positions;
    int num_positions;
} Dataset;



void add_dataset_to_file(const char *filename, Dataset *ds) {
    FILE *fp = fopen(filename, "a");
    if (!fp) {
        perror("Failed to open file for writing");
        return;
    }
    for (int i = 0; i < ds->num_positions; i++) {
        fprintf(fp, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                ds->positions[i].features[0], ds->positions[i].features[1],
                ds->positions[i].features[2], ds->positions[i].features[3],
                ds->positions[i].features[4], ds->positions[i].features[5],
                ds->positions[i].features[6], ds->positions[i].features[7],
                ds->positions[i].features[8], ds->positions[i].features[9],
                ds->positions[i].features[10], ds->positions[i].features[11],
                ds->positions[i].target);
    }
    fprintf(fp, "-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1\n"); // End of dataset marker
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


int line_counter = 1000;
double simulate_number_of_games(HashTable *ht, char depth, int number_of_games, char is_new_game) {
    /* Simulate a number of games and return the average score */
    // This version uses a random starting position from test_positions.txt
    double total_score = 0.0;
    depth_by_color[0] = depth;
    depth_by_color[1] = depth; 
    double score;
    int j; 
    Dataset *ds = malloc(sizeof(Dataset));
    ds->positions = malloc(sizeof(Position) * 10000);
    ds->num_positions = 0;
    double features[12];
    char boardstrings[MAX_POSSIBLE_MOVES][64];
    for (int i = 0; i < number_of_games; i++) {
        game the_game;
        memset(&the_game, 0, sizeof(game));
        board init;
        memset(&init, 0, sizeof(board));
        init_empty_board(&init);
        char boardstring[100];
        int i = 0;
        line_counter++;
        FILE *fp = fopen("shuffled_boardstrings.txt", "r");
        while (i < line_counter) {
            fgets(boardstring, sizeof(boardstring), fp);
            i++;
        }
        boardstring[61] = rand() % 2 == 0 ? 'W' : 'B';
        boardstring[62] = '\0';
        board_string_to_board(&init, boardstring);
        if (is_new_game) {
            memset(&init, 0, sizeof(board));
            init_empty_board(&init);
            belgian_daisy_opening(&init);
        }
        create_game(&the_game, &init);

        for (j = 0; j < MAX_POSSIBLE_MOVES; j++) {
            get_features2(the_game.current_position, the_game.current_position->whose_turn, features);
            get_board_string(the_game.current_position, boardstrings[j]);
            memcpy(ds->positions[ds->num_positions].features, features, sizeof(ds->positions[ds->num_positions].features));
            ds->num_positions++;
            int temp = bot_move(&the_game, ht, -1);
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
    if (!is_new_game) {
        add_dataset_to_file("all_games_depth_2.csv", ds);
        save_boardstring_to_file("all_boardstrings_2.txt", boardstrings, j);
    }
    return total_score;
}


void selfplay(HashTable *ht, int number_of_games, char depth) {
    // This function plays a number of games against itself and saves the features to a file
    for (int i = 0; i < number_of_games; i++) {
        eval_function_by_color[0] = 2;
        eval_function_by_color[1] = 2;
        double score = simulate_number_of_games(ht, depth, 1, 0);
        ht_clear(ht);
    }
}

void get_features_and_evals_from_boardstrings_file(const char *input_filename, const char *output_filename) {
    FILE *fp = fopen(input_filename, "r");
    if (!fp) {
        perror("Failed to open input file");
        return;
    }
    char line[100];
    FILE *out_fp = fopen(output_filename, "w");
    if (!out_fp) {
        perror("Failed to open output file");
        fclose(fp);
        return;
    }
    board b;
    double features[12];
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        line[61] = 'B';
        line[62] = '\0';
        board_string_to_board(&b, line);
        get_features2(&b, b.whose_turn, features);
        fprintf(out_fp, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
                features[0], features[1], features[2], features[3],
                features[4], features[5], features[6], features[7],
                features[8], features[9], features[10], features[11]);
        evaluation_function_number = 2;
        double eval = evaluate(&b, b.whose_turn);
        fprintf(out_fp, ",%lf\n", eval);
        count++;
        if (count % 1000 == 0) {
            printf("Processed %d positions\n", count);
            fflush(stdout);
        }
    }
}

void get_intresting_positions() {
    // use all_boardstrings.txt to get the eval of each position and if it's close to 0, save it to a file (test_positions.txt)
    FILE *fp = fopen("all_boardstrings.txt", "r");
    if (!fp) {
        perror("Failed to open input file");
        return;
    }
    char line[100];
    FILE *out_fp = fopen("test_positions.txt", "w");
    if (!out_fp) {
        perror("Failed to open output file");
        fclose(fp);
        return;
    }
    board b;
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        line[61] = 'B';
        line[62] = '\0';
        board_string_to_board(&b, line);
        evaluation_function_number = 2;
        double eval = evaluate(&b, b.whose_turn);
        if (eval > -0.1 && eval < 0.1 && rand() % 10 == 0) {
            fprintf(out_fp, "%s\n", line);
            count++;
        }
        if (count % 1000 == 0) {
            printf("Found %d intresting positions\n", count);
            fflush(stdout);
        }
    }
}

#define MAX_SETS 10
#define RATING_FILE "ratings.txt"

// ELO update constants
#define K 32

// Load ratings if file exists, otherwise initialize to 1000
void load_ratings(double *ratings, int num_sets) {
    FILE *f = fopen(RATING_FILE, "r");
    if (f) {
        for (int i = 0; i < num_sets; i++) {
            if (fscanf(f, "%lf", &ratings[i]) != 1) {
                ratings[i] = 1000.0;
            }
        }
        fclose(f);
    } else {
        for (int i = 0; i < num_sets; i++)
            ratings[i] = 1000.0;
    }
}

// Save ratings to file
void save_ratings(double *ratings, int num_sets) {
    FILE *f = fopen(RATING_FILE, "w");
    if (!f) {
        perror("Failed to save ratings");
        return;
    }
    for (int i = 0; i < num_sets; i++)
        fprintf(f, "%.1f\n", ratings[i]);
    fclose(f);
}

#include <sys/stat.h>
#include <time.h>

void assign_elo_for_file(const char *filename, int num_sets, HashTable *ht) {
    double weights[MAX_SETS][WEIGHT_COUNT];
    double ratings[MAX_SETS];

    // Record file modification time at start
    struct stat file_stat_start;
    if (stat(filename, &file_stat_start) != 0) {
        perror("stat failed");
        return;
    }
    time_t mtime_start = file_stat_start.st_mtime;

    // Load existing ratings or initialize
    load_ratings(ratings, num_sets);

    // Load all weight sets
    for (int i = 0; i < num_sets; i++) {
        if (load_weights_set(filename, weights[i], WEIGHT_COUNT, i) != 0) {
            fprintf(stderr, "Failed to load set %d\n", i);
            return;
        }
    }

    eval_function_by_color[0] = 2;
    eval_function_by_color[1] = 1;

    // Round-robin matches
    for (int i = 0; i < num_sets; i++) {
        for (int j = i + 1; j < num_sets; j++) {
            // Player i as white, j as black
            memcpy(self_play_weights1, weights[i], sizeof(double) * WEIGHT_COUNT);
            memcpy(self_play_weights2, weights[j], sizeof(double) * WEIGHT_COUNT);
            double score_i_white = (simulate_number_of_games(ht, 2, 1, 1) + 1) / 2.0;

            // Player j as white, i as black
            memcpy(self_play_weights1, weights[j], sizeof(double) * WEIGHT_COUNT);
            memcpy(self_play_weights2, weights[i], sizeof(double) * WEIGHT_COUNT);
            double score_j_white = (simulate_number_of_games(ht, 2, 1, 1) + 1) / 2.0;

            // Compute ELO update
            double expected_i = 1.0 / (1.0 + pow(10.0, (ratings[j] - ratings[i]) / 400.0));
            double actual_i = (score_i_white + (1 - score_j_white)) / 2.0;
            double actual_j = 1.0 - actual_i;

            ratings[i] += K * (actual_i - expected_i);
            ratings[j] += K * (actual_j - (1.0 / (1.0 + pow(10.0, (ratings[i] - ratings[j]) / 400.0))));
        }
    }

    // Check if file changed during execution
    struct stat file_stat_end;
    if (stat(filename, &file_stat_end) != 0) {
        perror("stat failed");
        return;
    }
    time_t mtime_end = file_stat_end.st_mtime;

    if (mtime_start != mtime_end) {
        fprintf(stderr, "Warning: file %s changed during execution. Ratings not saved.\n", filename);
        return;
    }

    // Safe to save ratings
    save_ratings(ratings, num_sets);

    // Print ratings
    for (int i = 0; i < num_sets; i++)
        printf("Set %d: ELO = %.1f\n", i, ratings[i]);
}


void add_new_set_and_estimate_elo(const char *main_file, const char *new_set_file, int total_sets, HashTable *ht) {
    if (total_sets < 1) {
        fprintf(stderr, "Need at least one existing set to compare.\n");
        return;
    }

    double weights[MAX_SETS][WEIGHT_COUNT];
    double ratings[MAX_SETS];

    // Load existing ratings
    load_ratings(ratings, total_sets);

    // Load existing sets
    for (int i = 0; i < total_sets; i++) {
        if (load_weights_set(main_file, weights[i], WEIGHT_COUNT, i) != 0) {
            fprintf(stderr, "Failed to load set %d from main file\n", i);
            return;
        }
    }

    // Load new set from separate file into last index
    int new_index = total_sets;  // temporarily store in next slot
    if (load_weights_set(new_set_file, weights[new_index], WEIGHT_COUNT, 0) != 0) {
        fprintf(stderr, "Failed to load new set from %s\n", new_set_file);
        return;
    }

    // Compute ELO for new set
    double new_elo = 0.0;
    double total_expected = 0.0;

    eval_function_by_color[0] = 2;
    eval_function_by_color[1] = 1;

    int num_games_per_match = 5;

    for (int i = 0; i < total_sets; i++) {
        // New set as white, old as black
        memcpy(self_play_weights1, weights[new_index], sizeof(double) * WEIGHT_COUNT);
        memcpy(self_play_weights2, weights[i], sizeof(double) * WEIGHT_COUNT);
        int score_white = (simulate_number_of_games(ht, 2, num_games_per_match, 1) + num_games_per_match) / 2.0;

        // New set as black, old as white
        memcpy(self_play_weights1, weights[i], sizeof(double) * WEIGHT_COUNT);
        memcpy(self_play_weights2, weights[new_index], sizeof(double) * WEIGHT_COUNT);
        int score_black = (simulate_number_of_games(ht, 2, num_games_per_match, 1) + num_games_per_match) / 2.0;

        double score_as_white = (double)score_white / num_games_per_match;
        double score_as_black = 1.0 - ((double)score_black / num_games_per_match);
        double actual_score = (score_as_white + score_as_black) / 2.0;
        if (actual_score >= 1) actual_score = 1 - 0.0001;
        if (actual_score <= 0) actual_score = 0.0001;

        double elo_diff = -400.0 * log10((1.0 / actual_score) - 1.0);
        new_elo += ratings[i] + elo_diff;
        total_expected += 1.0;
    }

    new_elo /= total_expected;

    printf("New set's elo: %.1f\n", new_elo);

    // Check if it's the best 
    int is_best = 1; 
    int max = 0;
    for (int i = 0; i < total_sets; i++) { 
        if (ratings[i] > max)
            max = ratings[i];
        if (ratings[i] >= new_elo) {
            is_best = 0; 
            break; 
        } 
    }
    if (is_best) {
        printf("NEW BEST\n");
        new_elo = MIN(new_elo, max + 250);
    }

    // Find worst existing set
    int worst_index = 0;
    for (int i = 1; i < total_sets; i++) {
        if (ratings[i] < ratings[worst_index]) worst_index = i;
    }

    if (new_elo > ratings[worst_index]) {
        printf("New set from %s is better than worst (%.1f). Replacing set %d.\n",
               new_set_file, ratings[worst_index], worst_index);

        // Overwrite worst set with new set
        memcpy(weights[worst_index], weights[new_index], sizeof(double) * WEIGHT_COUNT);
        ratings[worst_index] = new_elo;

        // Rewrite main weights file
        FILE *f = fopen(main_file, "w");
        if (!f) {
            perror("Failed to rewrite main weights file");
            return;
        }
        for (int i = 0; i < total_sets; i++) {
            for (int j = 0; j < WEIGHT_COUNT; j++)
                fprintf(f, "%.10f\n", weights[i][j]);
            if (i < total_sets - 1) fprintf(f, "\n"); // empty line between sets
        }
        fclose(f);
    } else {
        printf("New set is not better than the worst (%.1f). Discarded.\n", ratings[worst_index]);
    }

    save_ratings(ratings, total_sets);

    // Print ratings
    for (int i = 0; i < total_sets; i++)
        printf("Set %d ELO: %.1f\n", i, ratings[i]);
}


char is_comparing = 0;

/* This function parses the commands. */
char uci_parse(game *the_game, char is_game_on, HashTable *ht)
{       
    char line[10000];
    board *init = malloc(sizeof(board));
    memset(init, 0, sizeof(board));
    fgets (line, 8192, stdin);  
    
    if (!strncmp (line, "newgame", 6)){
        //eval_function_by_color[0] = 4;
        //eval_function_by_color[1] = 4;
        // the next 2 chars are the depths
        char depth1 = line[8];
        char depth2 = line[10]; 
        depth_by_color[0] = depth1 - '0';
        depth_by_color[1] = depth2 - '0';
        ht_clear(ht);
        init_empty_board(init);
        if (line[12] == 'B')
            belgian_daisy_opening(init);
        else if (line[12] == 'R')
            regualr_opening(init);
        else if (line[12] == 'D')
            debug_opening(init);
        else {
            printf("Unknown opening: %s\n", line + 12);
            belgian_daisy_opening(init); // Default to daisy opening
        }
        create_game(the_game,init);
        is_game_on = 1;
        print_board_string(the_game->current_position);
    }

    if (!strncmp (line, "position", 8))
	{
		char *posline = line + 9;

		if (!strncmp (posline, "startpos", 8))
		{
            init_empty_board(init);
            belgian_daisy_opening(init);
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
                depth_by_color[(the_game->current_position->whose_turn)] = line[3] - '0';
            bot_move(the_game, ht,1);
            print_board_string(the_game->current_position);
            //print_board(the_game->current_position);
            if (!check_endgame(the_game)) {
                is_game_on = 0;
            }
        }
        else {
            printf("Game not started. (use newgame to start a new game)\n");
        }
	}
    if (!strncmp (line, "simulate", 7))
    {
        if (is_game_on) {
            while (1) {
                bot_move(the_game, ht,0);
                printf("move number: %d\n", the_game->number_of_moves_in_game);
                if (the_game->tc.increment[0] != -1)
                    printf("White time left: %.2f seconds, Black time left: %.2f seconds\n", the_game->tc.time_left[0], the_game->tc.time_left[1]);
                if (!check_endgame(the_game)) {
                    is_game_on = 0;
                    break;
                }
            }
        }
        else {
            printf("Game not started. (use newgame to start a new game)\n");
        }
    }
    if (!strncmp (line, "selfplay", 8))
    {
        is_comparing = 0;
        selfplay(ht, 700, 4);
        exit(0);
    }
    if (!strncmp (line, "compare", 7))
    {
        is_comparing = 1;
        add_new_set_and_estimate_elo("weights.txt", "weights_A.txt", 4, ht);
        exit(0);
    }
    if (!strncmp (line, "elo", 3))
    {
        is_comparing = 1;
        while (1) {
            assign_elo_for_file("weights.txt", 4, ht);
            ht_clear(ht);
        }
    }
    if (!strncmp (line, "analyse", 7)) {
        is_comparing = 0;
        char depth = 4;
        if (line[8] != '\0') 
            depth = line[8] - '0';
        if (is_game_on)
            print_analysis(the_game, depth, 3, ht);
    }
    if (!strncmp (line, "move", 4))
    {
        char *posline = line + 5;
        player_move(the_game,posline);
        print_board(the_game->current_position);
        print_board_string(the_game->current_position);
        check_endgame(the_game);
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
            printf("Game not started. (use newgame to start a new game)\n");
        }
    }
    if (!strncmp (line, "quit", 4)) {
        exit(0);
    }

    return is_game_on;
}

int find_pazzles(const char *filename, HashTable *ht) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: could not open %s\n", filename);
        return -1;
    }
    FILE *fp_out = fopen("4d_pazzles.txt", "a");
    if (!fp_out) {
        fprintf(stderr, "Error: could not open %s\n", "4d_pazzles.txt");
        return -1;
    }
    eval_function_by_color[0] = 2;
    eval_function_by_color[1] = 2;
    evaluation_function_number = 2;
    char boardstring[128];
    int line_num = 0;
    char color;
    while (fgets(boardstring, sizeof(boardstring), fp)) {
        // Ensure boardstring is properly terminated
        boardstring[61] = rand() % 2 ? 'W' : 'B';
        color = boardstring[61] == 'W' ? WHITE : BLACK;
        boardstring[62] = '\0';

        board b;
        game g;
        memset(&b, 0, sizeof(board));
        memset(&g, 0, sizeof(game));
        init_empty_board(&b);
        board_string_to_board(&b, boardstring);
        create_game(&g, &b);

        double eval = evaluate(&b, color);
        eval *= color == BLACK ? 1 : -1;
        eval = print_analysis(&g, 4, 2, ht);
        if (eval) {
            print_board(&b);
            fprintf(fp_out, "%s\n", boardstring);
            fflush(fp_out);
        }
        line_num++;
        if (line_num % 100 == 0)
            printf("calculated so far %d positions\n", line_num);
    }

    fclose(fp);
    return line_num;  // return number of boardstrings loaded
}

int main()
{
    
	setbuf (stdin, NULL);
	setbuf (stdout, NULL);

	printf ("\nid name Gnizabalone\n");
	printf ("id author Elchairoy Meir\n");
	fflush (stdout);

    HashTable ht;
    game *the_game = calloc(1, sizeof(*the_game));
    char is_game_on = 0;
    
    /* Initialize ht: */ 
    ht_setup(&ht,sizeof(ht_board_struct),sizeof(ht_move_eval_struct),HT_CAPACITY);

    load_weights_set("weights_A.txt", self_play_weights1, WEIGHT_COUNT, 0);
    load_weights_set("weights_B.txt", self_play_weights2, WEIGHT_COUNT, 0); // still best
    load_weights_set("weights_C.txt", self_play_weights3, WEIGHT_COUNT, 0);
    is_comparing = 0;
    eval_function_by_color[0] = 2;
    eval_function_by_color[1] = 2;
    srand(time(NULL)); // Seed the random number generator
    //add_new_set_and_estimate_elo("weights.txt", "weights_A.txt", 4, &ht);
    //find_pazzles("slightly_worse.txt", &ht);

	while(1) {
        is_game_on = uci_parse(the_game, is_game_on, &ht);
    }
    
	return 0;
}



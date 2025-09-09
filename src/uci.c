#include "../include/uci.h"
#include "evaluation.h"

extern long int number_of_moves;
extern long int number_of_ht_found;
extern char evaluation_function_number;

extern int history_heuristic_push[2*RADIUS - 1][2*RADIUS - 1][6];
extern int history_heuristic_aside[2*RADIUS - 1][2*RADIUS - 1][6][2*RADIUS - 1][2*RADIUS - 1];

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
extern char NULL_MOVE_REDUCTION;
extern char MIN_NULL_MOVE;
extern clock_t start_time;
extern double max_duration;
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
            printf("best so far: "); print_move(best_move.m);
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

    commit_a_move_in_game(the_game, best_move.m);

    // Optional logging and hash table maintenance
    if (logs != -1) { 
        printf("eval: %lf\n", best_move.eval);
        print_board(the_board);
    }
    if(ht->size >= 2000000) { ht_clear(ht); printf("Hash table cleared\n"); }

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
        if (the_game->tc.time_left[1] <= 0) {
            printf("WHITE WON ON TIMEOUT\n");
            fflush(stdout);
            return 0;
        }
        if (all_moves[0] == END) {
            printf("STALMATE 0.5-0.5\n");
            fflush(stdout);
            return 0;
        }
        if (check_repetition(the_game, 0) || the_game->number_of_moves_in_game >= 300) {
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
        if (the_game->tc.time_left[0] <= 0) {
            printf("BLACK WON ON TIMEOUT\n");
            fflush(stdout);
            return 0;
        }
        if (all_moves[0] == END) {
            printf("STALMATE 0.5-0.5\n");
            fflush(stdout);
            return 0;
        }
        if (check_repetition(the_game, 0) || the_game->number_of_moves_in_game >= 300) {
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
        else if (the_game->tc.time_left[1] <= 0) {
            return -1;
        }
        else if (all_moves[0] == END || check_repetition(the_game, 0) || the_game->number_of_moves_in_game >= 300) {
            return 0;
        }
    }
    else
    {
        get_possible_moves(the_board,all_moves);
        if (is_lost(the_board, BLACK)) {
            return 1;
        }
        else if (the_game->tc.time_left[0] <= 0) {
            return 1;
        }
        else if (all_moves[0] == END || check_repetition(the_game, 0) || the_game->number_of_moves_in_game >= 300) {
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
    change_the_square(b, -4, 4, white_marble);

    b->hash = _ht_default_hash(b);
}

void debug_opening(board *b) {
    b->whose_turn = BLACK;
    // start with all black marbles in the center, and the white is in the corners
    char *bs = "W00000B0B0W00000WW00BBBWWW000BBBWWW000BB000000B0000BB00000W00B";
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
    g->initial_position.hash = initial_position->hash;

    /* Default time control: */
    g->tc.time_left[0] = 100;
    g->tc.time_left[1] = 100;
    g->tc.increment[0] = -1;
    g->tc.increment[1] = -1;
}

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
                depth_by_color[!(the_game->current_position->whose_turn)] = line[3] - '0';
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


int line_counter = 0;
double simulate_number_of_games(HashTable *ht, char depth, int number_of_games) {
    /* Simulate a number of games and return the average score */
    // This version uses a random starting position from test_positions.txt
    game the_game;
    memset(&the_game, 0, sizeof(game));
    board init;
    memset(&init, 0, sizeof(board));
    init_empty_board(&init);
    char boardstring[100];
    int i = 0;
    line_counter++;
    FILE *fp = fopen("test_positions.txt", "r");
    while (i < line_counter) {
        fgets(boardstring, sizeof(boardstring), fp);
        i++;
    }
    boardstring[61] = rand() % 2 == 0 ? 'W' : 'B';
    boardstring[62] = '\0';
    board_string_to_board(&init, boardstring);
    create_game(&the_game, &init);
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
        char temp = eval_function_by_color[0];
        eval_function_by_color[0] = eval_function_by_color[1];
        eval_function_by_color[1] = temp;
        temp = depth_by_color[0];
        depth_by_color[0] = depth_by_color[1];
        depth_by_color[1] = temp;
        for (j = 0; j < MAX_POSSIBLE_MOVES; j++) {
            get_features2(the_game.current_position, the_game.current_position->whose_turn, features);
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
    add_dataset_to_file("all_games_depth_2.csv", ds);
    save_boardstring_to_file("all_boardstrings_2.txt", boardstrings, j);
    return total_score / number_of_games;
}


void selfplay(HashTable *ht, int number_of_games, char depth) {
    // This function plays a number of games against itself and saves the features to a file
    for (int i = 0; i < number_of_games; i++) {
        eval_function_by_color[0] = 5;
        eval_function_by_color[1] = 5;
        double score = simulate_number_of_games(ht, depth, 1);
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
        evaluation_function_number = 5;
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
        evaluation_function_number = 5;
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

extern double self_play_weights1[WEIGHT_COUNT];
extern double self_play_weights2[WEIGHT_COUNT];
extern double self_play_weights3[WEIGHT_COUNT];


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
    ht_setup(&ht,sizeof(ht_board_struct),sizeof(ht_move_eval_struct),2000000);

    load_weights("weights_A.txt", self_play_weights1, WEIGHT_COUNT);
    load_weights("weights_B.txt", self_play_weights2, WEIGHT_COUNT); // best one.
    load_weights("weights_C.txt", self_play_weights3, WEIGHT_COUNT);
    eval_function_by_color[0] = 5;
    eval_function_by_color[1] = 5;
    srand(time(NULL)); // Seed the random number generator
    //selfplay(&ht, 10000, 2);
    //get_intresting_positions();
     
	while(1) {
        is_game_on = uci_parse(the_game, is_game_on, &ht);
    }
    
	return 0;
}



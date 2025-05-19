#include "../include/uci.h"
#include "evaluation.h"

extern long int number_of_moves;
extern long int number_of_ht_inserted;
long long int number_of_moves_test = 0;
extern char evaluation_function_number;

extern int history_heuristic_push[2*RADIUS - 1][2*RADIUS - 1][6];
extern int history_heuristic_aside[2*RADIUS - 1][2*RADIUS - 1][6][2*RADIUS - 1][2*RADIUS - 1];


extern double weights[7][4];
extern double poly_coefficients_1[7][4];
extern double poly_coefficients_2[7][4];

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
    
    char src_label[3], dst_label[3], end_of_line_label[3];
    char *src_square_str = my_strndup(move_str, 2);
    char *dst_square_str = my_strndup(move_str + 2, 2);
    char end_of_line_row, end_of_line_col;
    if (move_str[4] == '\0')
        end_of_line_label[0] = '\0';
    else {
        char *end_of_line_square_str = my_strndup(move_str + 4, 2);
        char end_of_line_cord[2];
        memcpy(end_of_line_cord, label_to_coord(end_of_line_square_str), 2);
        end_of_line_row = end_of_line_cord[0], end_of_line_col = end_of_line_cord[1];
    }

    char src_cord[2];
    memcpy(src_cord, label_to_coord(src_square_str), 2);
    char src_row = src_cord[0], src_col = src_cord[1];
    char dst_cord[2];
    memcpy(dst_cord, label_to_coord(dst_square_str), 2);
    char dst_row = dst_cord[0], dst_col = dst_cord[1];
    char d = get_direction_between_squares(src_row, src_col, dst_row, dst_col);

    get_possible_moves(the_board, all_moves, 0, 0, 0);
    for (int i = 0; all_moves[i] != END; i++)
    {
        //print_move(all_moves[i]);
        if (((get_src_row(all_moves[i]) == src_row && get_src_col(all_moves[i]) == src_col)||(get_src_row(all_moves[i]) == end_of_line_row && get_src_col(all_moves[i]) == end_of_line_col)) && get_direction(all_moves[i]) == d) {
            if (get_move_type(all_moves[i]) == ASIDE && end_of_line_label[0] != '\0') {
                if (((get_end_of_line_row(all_moves[i]) == end_of_line_row && get_end_of_line_col(all_moves[i]) == end_of_line_col) || (get_end_of_line_row(all_moves[i]) == src_row && get_end_of_line_col(all_moves[i]) == src_col))) {
                    temp = all_moves[i];
                    break;
                }
            }
            else if (get_move_type(all_moves[i]) == STRAIGHT && end_of_line_label[0] == '\0') {
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
        printf("invalid move\n");
        return 0;
    }
}


char eval_function_by_color[2] = {2, 2};
char depth_by_color[2] = {3, 3};
extern char NULL_MOVE_REDUCTION;

double bot_move(game *the_game, HashTable *ht, char logs)
{
    HashTable h;
    if(ht->size >= 20000000) {
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
        if (eval_function_by_color[0] == 1) {
            memcpy(weights, poly_coefficients_1, sizeof(weights));
        }
        else if (eval_function_by_color[0] == 2) {
            memcpy(weights, poly_coefficients_2, sizeof(weights));
        }
        NULL_MOVE_REDUCTION = 3;
        evaluation_function_number = eval_function_by_color[0];
        move possible_moves[MAX_POSSIBLE_MOVES];
        get_possible_moves(the_board, possible_moves, 0, 0, 0);
        int no_possible_moves = 0;
        while (possible_moves[no_possible_moves] != END)
            no_possible_moves++;
        double last_depth_time = 0.0000001;
        while (i <= depth_by_color[0]){
            double time_forcast_for_next_depth = pow(no_possible_moves, 0.75*i) / 300000;
            if (logs == 1)
                printf("time_forcast_for_next_depth: %lf\n", time_forcast_for_next_depth);
            bot_move = get_best_move_white(the_game, i, bot_move.eval - ASPIRATION_WINDOW, bot_move.eval + ASPIRATION_WINDOW, ht);
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
        if (logs!=-1) {
            printf("bestmove ");
            print_move(m);
            printf(" ");
        }
        commit_a_move_in_game(the_game, bot_move.m);
    }
    
    else
    {
        if (eval_function_by_color[1] == 1) {
            memcpy(weights, poly_coefficients_1, sizeof(weights));
        }
        else if (eval_function_by_color[1] == 2) {
            memcpy(weights, poly_coefficients_2, sizeof(weights));
        }
        NULL_MOVE_REDUCTION = 3;
        evaluation_function_number = eval_function_by_color[1];
        move possible_moves[MAX_POSSIBLE_MOVES];
        get_possible_moves(the_board, possible_moves, 0, 0, 0);
        int no_possible_moves = 0;
        while (possible_moves[no_possible_moves] != END)
            no_possible_moves++;
        double last_depth_time = 0.0000001;
        while (i <= depth_by_color[1]){
            double time_forcast_for_next_depth = pow(no_possible_moves, 0.75*i) / 300000;
            if (logs == 1)
                printf("time_forcast_for_next_depth: %lf\n", time_forcast_for_next_depth);
            //if (i >= min_depth && last_depth_time > 0.3)
             //   break;
            bot_move = get_best_move_black(the_game, i, bot_move.eval - ASPIRATION_WINDOW, bot_move.eval + ASPIRATION_WINDOW, ht);
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
        if (logs!=-1) {
            printf("bestmove ");
            print_move(m);
            printf(" ");
        }
        //printf("Eval: %lf\n",evaluate_minimax_for_black(the_game, 0, the_game->moves[no_of_moves-1],get_irrev_move_info(the_game->current_position, the_game->moves[no_of_moves-1]),depth,-10000,10000));
        commit_a_move_in_game(the_game, bot_move.m);   
    }
    
    if (logs != -1)  {
        printf("eval: %lf\n", bot_move.eval);
        print_board(the_board);}
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
            return 0;
        }
        else if (all_moves[0] == END) {
            printf("STALMATE 0.5-0.5\n");
            return 0;
        }
        if (check_repetition(the_game) || the_game->number_of_moves_in_game >= 300) {
            printf("REPETITION 0.5-0.5\n");
            return 0;
        }
    }
    else
    {
        get_all_moves_by_calculating_everything(the_board,all_moves);
        if (is_lost(the_board, BLACK)) {
            printf("WHITE WON\n");
            return 0;
        }
        else if (all_moves[0] == END) {
            printf("STALMATE 0.5-0.5\n");
            return 0;
        }
        if (check_repetition(the_game) || the_game->number_of_moves_in_game >= 300) {
            printf("REPETITION 0.5-0.5\n");
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
            return 0;
        }
    }
    else
    {
        get_all_moves_by_calculating_everything(the_board,all_moves);
        if (is_lost(the_board, BLACK)) {
            return 1;
        }
        else if (all_moves[0] == END || check_repetition(the_game) || the_game->number_of_moves_in_game >= 200) {
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
    b->whose_turn = WHITE;
    change_the_square(b, -3, 3, black_marble);
    change_the_square(b, -2, 2, black_marble);
    change_the_square(b, -4, 4, black_marble);
    
    change_the_square(b, 0, -0, white_marble);
    change_the_square(b, 1, -1, white_marble);
    change_the_square(b, 2, -2, white_marble);
    change_the_square(b, 3, -3, white_marble);
    change_the_square(b, -1, 1, white_marble);

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

int main()
{
	setbuf (stdin, NULL);
	setbuf (stdout, NULL);

	printf ("\nid name Gnizabalon\n");
	printf ("id author Elchai\n");
	printf ("uciok\n");
	fflush (stdout);

    HashTable ht;
    game *the_game = calloc(1, sizeof(*the_game));
    char is_game_on = 0;

    
    /* Initialize ht: */ 
    ht_setup(&ht,sizeof(ht_board_struct),sizeof(ht_move_eval_struct),1000000);
/*
    init_empty_board(&init);
    debug_opening(&init);
    create_game(the_game,&init);
    move m;
    irreversible_move_info inf;
    inf = get_irrev_move_info(the_game->current_position,m);
    move all_moves[200];
    get_possible_moves(the_game->current_position,all_moves,0,0,0);
    for (int i = 0; all_moves[i] != END; i++) {
        print_move(all_moves[i]);
    }
    
    print_board(the_game->current_position);
*/
	while(1) {
        is_game_on = uci_parse(the_game, is_game_on, &ht);
    }
    
	return 0;
}

double simulate(HashTable *ht, char depth, Dataset *ds) {
    game the_game;
    board init;

    init_empty_board(&init);
    begian_daisy_opening(&init);
    create_game(&the_game,&init);
    int no_of_moves = 0;
    int i = 0;
    depth_by_color[0] = depth;
    depth_by_color[1] = depth;
    double s;
    double feature_vector[8];
    Dataset temp_ds;
    temp_ds.num_positions = 0;
    while (1) {
        bot_move(&the_game, ht, -1);
        get_features(the_game.current_position, the_game.current_position->whose_turn, temp_ds.positions[temp_ds.num_positions].features);
        temp_ds.num_positions++;
        if (check_endgame(&the_game) == 0) {
            break;
        }
    }
    double score = who_won(&the_game);
        // copy all the temp positions to the dataset
    for (int i = 0; i < temp_ds.num_positions; i++) {
        memcpy(ds->positions[ds->num_positions].features, temp_ds.positions[i].features, sizeof(temp_ds.positions[i].features));
        ds->positions[ds->num_positions].target = score;
        ds->num_positions++;
    }

    return score;
}

char line[10000];
/* This function parses the commands. */
char uci_parse(game *the_game, char is_game_on, HashTable *ht)
{   
    /* check if ht is full: */
    ht_board_struct t1;
    ht_move_eval_struct t2;
    memset(&t1, 0, sizeof(ht_board_struct));
    memset(&t2, 0, sizeof(ht_move_eval_struct));
    
    board *init = malloc(sizeof(board));
    memset(init, 0, sizeof(board));
    fgets (line, 8192, stdin);  

	if (!strncmp (line, "isready", 7))
		printf ("readyok\n");

    if (!strncmp (line, "ucinewgame", 10)){
        // the next 2 chars are the depths
        char depth1 = line[11];
        char depth2 = line[13]; 
        depth_by_color[0] = depth1 - '0';
        depth_by_color[1] = depth2 - '0';
        ht_clear(ht);
        init_empty_board(init);
        begian_daisy_opening(init);
        create_game(the_game,init);
        is_game_on = 1;
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

				posline = strstr (posline, " ");

				if (!posline)
					break;

				posline ++;
                //print_board(the_game->current_position);
			}
		}
        if (!check_endgame(the_game)) {
            is_game_on = 0;
        }
	}
    if (!strncmp (line, "go", 2))
	{
        if (is_game_on) {
            //print_board(the_game->current_position);
            bot_move(the_game, ht,1);
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
    }
    if (!strncmp (line, "mg", 2))
    {
        char *posline = line + 3;
        
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


void moves_in_depth(char d,board *b,move *all_moves_last_move, move last_move, irreversible_move_info inf) {
    move all_moves[200];
    move m;
    int i;
    irreversible_move_info temp;

    number_of_moves_test++;

    if (d == 0)
        return;

    get_possible_moves(b,all_moves,all_moves_last_move, last_move, inf);
    
    if (b->whose_turn == WHITE)
        i = 0;
    else
        i = 100;
    m = all_moves[i];
    while (m != END)
    {
        temp = get_irrev_move_info(b,m);
        if (b->whose_turn == WHITE){
            commit_a_move_in_board(b,m);
            moves_in_depth(d-1,b,all_moves,m,temp);
            unmake_move_in_board(b,m,temp);
        }
        else {
            commit_a_move_in_board(b,m);
            moves_in_depth(d-1,b,all_moves,m,temp);
            unmake_move_in_board(b,m,temp);
        }
        i++;
        m = all_moves[i];
    }
    return;
}
#include "../include/evaluation.h"


#include <stdlib.h>
#include <math.h>

#define STARTING_MARBLES_WHITE 14
#define STARTING_MARBLES_BLACK 14

char evaluation_function_number = 0;


#define POLY(x, a1, a2, a3, a4) \
    (((x) >= 0) * (a1) + (a2) * (x) + ((x) >= 0) * (a3) * (x) * (x) + (a4) * (x) * (x) * (x))
    
#define ABS(x) ((x) < 0 ? -(x) : (x))


double self_play_weights1[WEIGHT_COUNT];
double self_play_weights2[WEIGHT_COUNT];
double self_play_weights3[WEIGHT_COUNT];


void get_features(board* b, char color, double* features) {
    double white_marbles = 0, black_marbles = 0;
    double proximity_score_white = 0, proximity_score_black = 0;
    double proximity_squared_score_white = 0, proximity_squared_score_black = 0;
    double cohesion_score_white = 0, cohesion_score_black = 0;
    double break_strong_group_score_white = 0, break_strong_group_score_black = 0;
    double strengthen_group_score_white = 0, strengthen_group_score_black = 0;
    double lines_of_3_white = 0, lines_of_3_black = 0;
    char white_center_row = 0, white_center_col = 0;
    char black_center_row = 0, black_center_col = 0;

    for (int i = -RADIUS + 1; i < RADIUS; i++) {
        for (int j = -RADIUS + 1; j < RADIUS; j++) {
            char marb = get_marb_in_square(b, i, j);

            if (marb == white_marble) {
                white_marbles++;
                white_center_row += i;
                white_center_col += j;

                int dist = hex_distance(i, j, 0, 0);
                proximity_score_white += RADIUS - dist - 1;
                proximity_squared_score_white += dist * dist * dist;
                cohesion_score_white += get_no_neighbours(b, i, j, WHITE);

                for (enum directions d = 0; d < 6; d++) {
                    char marb_in_dir = get_marb_dir1(b, i, j, d);
                    char marb_in_back_dir = get_marb_dir1(b, i, j, get_backward_direction(d));

                    if (marb_in_dir == black_marble) {
                        if (marb_in_back_dir == black_marble)
                            break_strong_group_score_white++;
                        else if (marb_in_back_dir == white_marble)
                            strengthen_group_score_white++;
                    }

                    if (marb_in_dir == white_marble && marb_in_back_dir == white_marble)
                        lines_of_3_white += RADIUS - distance_of_line_from_square(i, j, d, 0, 0);
                }
            }

            else if (marb == black_marble) {
                black_marbles++;
                black_center_row += i;
                black_center_col += j;

                int dist = hex_distance(i, j, 0, 0);
                proximity_score_black += RADIUS - dist - 1;
                proximity_squared_score_black += dist * dist * dist;
                cohesion_score_black += get_no_neighbours(b, i, j, BLACK);

                for (enum directions d = 0; d < 6; d++) {
                    char marb_in_dir = get_marb_dir1(b, i, j, d);
                    char marb_in_back_dir = get_marb_dir1(b, i, j, get_backward_direction(d));

                    if (marb_in_dir == white_marble) {
                        if (marb_in_back_dir == white_marble)
                            break_strong_group_score_black++;
                        else if (marb_in_back_dir == black_marble)
                            strengthen_group_score_black++;
                    }

                    if (marb_in_dir == black_marble && marb_in_back_dir == black_marble)
                        lines_of_3_black += RADIUS - distance_of_line_from_square(i, j, d, 0, 0);
                }
            }
        }
    }

    white_center_row /= white_marbles;
    white_center_col /= white_marbles;
    black_center_row /= black_marbles;
    black_center_col /= black_marbles;

    double white_proximity_to_itself = 0, black_proximity_to_itself = 0;
    double white_distant_marbles = 0, black_distant_marbles = 0;

    for (int i = -RADIUS + 1; i < RADIUS; i++) {
        for (int j = -RADIUS + 1; j < RADIUS; j++) {
            char marb = get_marb_in_square(b, i, j);

            if (marb == white_marble) {
                int dist = hex_distance(i, j, white_center_row, white_center_col);
                white_proximity_to_itself += RADIUS - dist - 1;
                if (dist >= 3) white_distant_marbles++;
            }
            else if (marb == black_marble) {
                int dist = hex_distance(i, j, black_center_row, black_center_col);
                black_proximity_to_itself += RADIUS - dist - 1;
                if (dist >= 3) black_distant_marbles++;
            }
        }
    }

    int f = 0;
    // White features
    features[f++] = proximity_score_white - proximity_score_black; // proximity score difference
    features[f++] = proximity_score_white / white_marbles - proximity_score_black / black_marbles; // normalized by number of marbles
    features[f++] = proximity_squared_score_white / white_marbles - proximity_squared_score_black / black_marbles; // normalized by number of marbles
    features[f++] = cohesion_score_white - cohesion_score_black;
    features[f++] = break_strong_group_score_white - break_strong_group_score_black;
    features[f++] = strengthen_group_score_white - strengthen_group_score_black;
    features[f++] = lines_of_3_white - lines_of_3_black;
    features[f++] = (STARTING_MARBLES_BLACK - black_marbles) - (STARTING_MARBLES_WHITE - white_marbles); // difference in marbles
    features[f++] = pow(2,proximity_score_white/white_marbles) * (STARTING_MARBLES_BLACK - black_marbles) -
                  pow(2,proximity_score_black/black_marbles) * (STARTING_MARBLES_WHITE - white_marbles);
    features[f++] = white_proximity_to_itself / white_marbles - black_proximity_to_itself / black_marbles;
    features[f++] = white_distant_marbles - black_distant_marbles;

    // Relative feature
    features[f++] = hex_distance(white_center_row, white_center_col, black_center_row, black_center_col);

    const double MAX_ABS_VALUES[12] = {34. ,2., 64, 47., 18., 18.,  80.,  6.,  20, 2.888889, 13.000000, 3.000000};    // Normalize features
    for (int i = 0; i < 12; i++) {
        features[i] /= MAX_ABS_VALUES[i];
    }
}


void get_features2(board* b, char color, double* features) {
    double white_marbles = 0, black_marbles = 0;
    double proximity_score_white = 0, proximity_score_black = 0;
    double proximity_squared_score_white = 0, proximity_squared_score_black = 0;
    double cohesion_score_white = 0, cohesion_score_black = 0;
    double break_strong_group_score_white = 0, break_strong_group_score_black = 0;
    double strengthen_group_score_white = 0, strengthen_group_score_black = 0;
    double lines_of_3_white = 0, lines_of_3_black = 0;
    double white_center_row = 0, white_center_col = 0;
    double black_center_row = 0, black_center_col = 0;

    for (int i = -RADIUS + 1; i < RADIUS; i++) {
        for (int j = -RADIUS + 1; j < RADIUS; j++) {
            char marb = get_marb_in_square(b, i, j);

            if (marb == white_marble) {
                white_marbles++;
                white_center_row += i;
                white_center_col += j;

                int dist = hex_distance(i, j, 0, 0);
                proximity_score_white += RADIUS - dist - 1;
                proximity_squared_score_white += dist * dist * dist;
                cohesion_score_white += get_no_neighbours(b, i, j, WHITE);

                for (enum directions d = 0; d < 6; d++) {
                    char marb_in_dir = get_marb_dir1(b, i, j, d);
                    char marb_in_back_dir = get_marb_dir1(b, i, j, get_backward_direction(d));

                    if (marb_in_dir == black_marble) {
                        if (marb_in_back_dir == black_marble)
                            break_strong_group_score_white++;
                        else if (marb_in_back_dir == white_marble)
                            strengthen_group_score_white++;
                    }

                    if (marb_in_dir == white_marble && marb_in_back_dir == white_marble)
                        lines_of_3_white += RADIUS - distance_of_line_from_square(i, j, d, 0, 0);
                }
            }

            else if (marb == black_marble) {
                black_marbles++;
                black_center_row += i;
                black_center_col += j;

                int dist = hex_distance(i, j, 0, 0);
                proximity_score_black += RADIUS - dist - 1;
                proximity_squared_score_black += dist * dist * dist;
                cohesion_score_black += get_no_neighbours(b, i, j, BLACK);

                for (enum directions d = 0; d < 6; d++) {
                    char marb_in_dir = get_marb_dir1(b, i, j, d);
                    char marb_in_back_dir = get_marb_dir1(b, i, j, get_backward_direction(d));

                    if (marb_in_dir == white_marble) {
                        if (marb_in_back_dir == white_marble)
                            break_strong_group_score_black++;
                        else if (marb_in_back_dir == black_marble)
                            strengthen_group_score_black++;
                    }

                    if (marb_in_dir == black_marble && marb_in_back_dir == black_marble)
                        lines_of_3_black += RADIUS - distance_of_line_from_square(i, j, d, 0, 0);
                }
            }
        }
    }

    white_center_row /= white_marbles;
    white_center_col /= white_marbles;
    black_center_row /= black_marbles;
    black_center_col /= black_marbles;

    double white_proximity_to_itself = 0, black_proximity_to_itself = 0;
    double white_distant_marbles = 0, black_distant_marbles = 0;

    for (int i = -RADIUS + 1; i < RADIUS; i++) {
        for (int j = -RADIUS + 1; j < RADIUS; j++) {
            char marb = get_marb_in_square(b, i, j);

            if (marb == white_marble) {
                int dist = hex_distance(i, j, white_center_row, white_center_col);
                white_proximity_to_itself += RADIUS - dist - 1;
                if (dist >= 3) white_distant_marbles++;
            }
            else if (marb == black_marble) {
                int dist = hex_distance(i, j, black_center_row, black_center_col);
                black_proximity_to_itself += RADIUS - dist - 1;
                if (dist >= 3) black_distant_marbles++;
            }
        }
    }

    int f = 0;
    // White features
    features[f++] = proximity_score_white - proximity_score_black; // proximity score difference
    features[f++] = proximity_score_white / white_marbles - proximity_score_black / black_marbles; // normalized by number of marbles
    features[f++] = proximity_squared_score_white / white_marbles - proximity_squared_score_black / black_marbles; // normalized by number of marbles
    features[f++] = cohesion_score_white - cohesion_score_black;
    features[f++] = break_strong_group_score_white - break_strong_group_score_black;
    features[f++] = strengthen_group_score_white - strengthen_group_score_black;
    features[f++] = lines_of_3_white - lines_of_3_black;
    features[f++] = (STARTING_MARBLES_BLACK - black_marbles) - (STARTING_MARBLES_WHITE - white_marbles); // difference in marbles
    features[f++] = (STARTING_MARBLES_BLACK - black_marbles) + (STARTING_MARBLES_WHITE - white_marbles); // total marbles lost
    features[f++] = pow(2,proximity_score_white/white_marbles) * (STARTING_MARBLES_BLACK - black_marbles) -
                  pow(2,proximity_score_black/black_marbles) * (STARTING_MARBLES_WHITE - white_marbles);
    features[f++] = white_proximity_to_itself / white_marbles - black_proximity_to_itself / black_marbles;
    features[f++] = white_distant_marbles - black_distant_marbles;


    const double MAX_ABS_VALUES[FEATURE_COUNT] = {34. ,2., 64, 47., 18., 18.,  80.,  6., 12., 20, 2.888889, 13.};    // Normalize features
    for (int i = 0; i < FEATURE_COUNT; i++) {
        features[i] /= MAX_ABS_VALUES[i];
    }
}

/***************************************************************************************************************************/
// ReLU activation
static inline double relu(double x) {
    return x > 0.0 ? x : 0.0;
}



// weights layout: 
// fc1_weights[HIDDEN_SIZE][INPUT_SIZE] (row-major)
// fc1_bias[HIDDEN_SIZE]
// fc2_weights[1][HIDDEN_SIZE] (just 8 weights)
// fc2_bias[1]
#define INPUT_SIZE 12
#define HIDDEN1_SIZE 16
#define HIDDEN2_SIZE 4

static inline double get_nn1_output(const double *input, const double *weights) {
    double hidden1[HIDDEN1_SIZE];
    double hidden2[HIDDEN2_SIZE];

    // --- fc1: input -> hidden1 with bias ---
    // weights layout: [fc1_weights | fc1_bias | fc2_weights | fc2_bias | fc3_weights | fc3_bias]
    int offset = 0;
    for (int j = 0; j < HIDDEN1_SIZE; j++) {
        double sum = 0.0;
        for (int i = 0; i < INPUT_SIZE; i++) {
            sum += input[i] * weights[offset + j * INPUT_SIZE + i];
        }
        // add bias
        sum += weights[offset + HIDDEN1_SIZE * INPUT_SIZE + j];
        hidden1[j] = tanh(sum);
    }

    offset += HIDDEN1_SIZE * INPUT_SIZE + HIDDEN1_SIZE;  // move past fc1 weights + bias

    // --- fc2: hidden1 -> hidden2 with bias ---
    for (int j = 0; j < HIDDEN2_SIZE; j++) {
        double sum = 0.0;
        for (int i = 0; i < HIDDEN1_SIZE; i++) {
            sum += hidden1[i] * weights[offset + j * HIDDEN1_SIZE + i];
        }
        // add bias
        sum += weights[offset + HIDDEN2_SIZE * HIDDEN1_SIZE + j];
        hidden2[j] = tanh(sum);
    }

    offset += HIDDEN2_SIZE * HIDDEN1_SIZE + HIDDEN2_SIZE;  // move past fc2 weights + bias

    // --- fc3: hidden2 -> output with bias ---
    double output = 0.0;
    for (int j = 0; j < HIDDEN2_SIZE; j++) {
        output += hidden2[j] * weights[offset + j];
    }
    // add output bias
    output += weights[offset + HIDDEN2_SIZE];

    return tanh(output);
}

// Another NN architecture
#define INPUT_SIZE2 12
#define HIDDEN1_SIZE2 16
#define HIDDEN2_SIZE2 4

static inline double get_nn2_output(const double *input, const double *weights) {
    double hidden1[HIDDEN1_SIZE2];
    double hidden2[HIDDEN2_SIZE2];

    // fc1: input -> hidden1, no bias, tanh
    for (int j = 0; j < HIDDEN1_SIZE2; j++) {
        double sum = 0.0;
        for (int i = 0; i < INPUT_SIZE2; i++) {
            sum += input[i] * weights[j * INPUT_SIZE2 + i];
        }
        hidden1[j] = tanh(sum);
    }

    // fc2: hidden1 -> hidden2, no bias, tanh
    int offset = HIDDEN1_SIZE2 * INPUT_SIZE2;
    for (int j = 0; j < HIDDEN2_SIZE2; j++) {
        double sum = 0.0;
        for (int i = 0; i < HIDDEN1_SIZE; i++) {
            sum += hidden1[i] * weights[offset + j * HIDDEN1_SIZE2 + i];
        }
        hidden2[j] = relu(sum);
    }

    // fc3: hidden2 -> output, no bias
    offset += HIDDEN2_SIZE2 * HIDDEN1_SIZE2;
    double output = 0.0;
    for (int j = 0; j < HIDDEN2_SIZE2; j++) {
        output += hidden2[j] * weights[offset + j];
    }

    return tanh(output);
}


double evaluate_nn1(board *b, char color) {
    double features[FEATURE_COUNT];
    get_features2(b, color, features);

    return get_nn1_output(features, self_play_weights1);
}


double evaluate_nn2(board *b, char color) {
    double features[FEATURE_COUNT];
    get_features2(b, color, features);
    double neg_features[FEATURE_COUNT];
    for (int i = 0; i < FEATURE_COUNT; i++) {
        neg_features[i] = -features[i];
    }
    neg_features[8] = features[8]; // total marbles lost is the same for both
    return (get_nn2_output(features, self_play_weights2) - get_nn2_output(neg_features, self_play_weights2)) / 2.0;
}

double evaluate_nn3(board *b, char color) {
    double features[12];
    get_features2(b, color, features);
    return (get_nn1_output(features, self_play_weights3));
}

/******************************************************************************************************************************************************************************* */

/* Evaluation function: returns score from white's perspective */
double evaluate(board* b, char color) {
    if (evaluation_function_number == 1) {
        return evaluate_nn1(b, color);
    }
    else if (evaluation_function_number == 2) {
        return evaluate_nn2(b, color);
    }
    else if (evaluation_function_number == 3) {
        return evaluate_nn3(b, color);
    }
    else {
        return 0.0;
    }
}

double evaluate_features(const double* features, char color) {
    return get_nn1_output(features, self_play_weights3);
}
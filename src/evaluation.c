#include "../include/evaluation.h"


#include <stdlib.h>
#include <math.h>

#define STARTING_MARBLES_WHITE 14
#define STARTING_MARBLES_BLACK 14

char evaluation_function_number = 0;

#define FEATURE_COUNT 9

#define POLY(x, a1, a2, a3, a4) \
    (((x) >= 0) * (a1) + (a2) * (x) + ((x) >= 0) * (a3) * (x) * (x) + (a4) * (x) * (x) * (x))
    
#define ABS(x) ((x) < 0 ? -(x) : (x))


double self_play_weights1[WEIGHT_COUNT];
double self_play_weights2[WEIGHT_COUNT];


double self_play_weights3[WEIGHT_COUNT] = {2.338444232940673828e+00, -1.675856262445449829e-01, -1.283129572868347168e+00, 2.762150764465332031e-01, 5.428210496902465820e-01, 8.639968037605285645e-01, 6.302828192710876465e-01, 1.013335466384887695e+00, 1.376662969589233398e+00, -2.576522827148437500e-01, -1.418987102806568146e-02, -1.682609319686889648e-02, -2.319136559963226318e-01, -1.849206387996673584e-01, -1.001169756054878235e-01, -8.047566562891006470e-02, 8.350002765655517578e-01, -8.572518229484558105e-01};
double self_play_weights4[WEIGHT_COUNT] = {9.417400956153869629e-01, -4.870436489582061768e-01, 8.253321051597595215e-01, 4.051494225859642029e-02, 5.573357939720153809e-01, 4.393408596515655518e-01, 1.515909790992736816e+00, 1.654362559318542480e+00, -3.269024193286895752e-01, -2.195937335491180420e-01, -4.986360967159271240e-01, 2.380724437534809113e-02, -9.388771653175354004e-02, -3.304605185985565186e-02, -5.442268215119838715e-03, -6.939247556945801e-02};

void get_features(board* b, char color, double* features) {
    double white_marbles = 0;
    double black_marbles = 0;
    double proximity_score_white = 0;
    double proximity_score_black = 0;
    double proximity_squared_score_white = 0;
    double proximity_squared_score_black = 0;
    double cohesion_score = 0;
    double break_strong_group_score = 0;
    double strengthen_group_score = 0;
    double lines_of_3 = 0;
    for (int i = -RADIUS + 1; i < RADIUS; i++) {
        for (int j = -RADIUS + 1; j < RADIUS; j++) {
            char marb = get_marb_in_square(b, i, j);
            if (marb == white_marble) {
                white_marbles++;
                int dist = hex_distance(i, j, 0, 0);
                proximity_score_white += RADIUS - dist - 1;
                proximity_squared_score_white += dist * dist * dist;
                cohesion_score += get_no_neighbours(b, i, j, WHITE);
                for (enum directions d = 0; d < 6; d++) {
                    char marb_in_dir = get_marb_dir1(b, i, j, d);
                    char marb_in_back_dir = get_marb_dir1(b, i, j, get_backward_direction(d));
                    if (marb_in_dir == black_marble) {
                        if (marb_in_back_dir == black_marble) {
                            break_strong_group_score ++;
                        }
                        else if (marb_in_back_dir == white_marble) {
                            strengthen_group_score ++;
                        }
                    }
                    if (marb_in_dir == white_marble) {
                        if (marb_in_back_dir == white_marble) {
                            lines_of_3++;
                        }
                    }
                }
            }
            else if (marb == black_marble) {
                black_marbles++;
                int dist = hex_distance(i, j, 0, 0);
                proximity_score_black += RADIUS - dist - 1;
                proximity_squared_score_black += dist * dist * dist;
                cohesion_score -= get_no_neighbours(b, i, j, BLACK);
                for (enum directions d = 0; d < 6; d++) {
                    char marb_in_dir = get_marb_dir1(b, i, j, d);
                    char marb_in_back_dir = get_marb_dir1(b, i, j, get_backward_direction(d));
                    if (marb_in_dir == white_marble) {
                        if (marb_in_back_dir == white_marble) {
                            break_strong_group_score -- ;
                        }
                        else if (marb_in_back_dir == black_marble) {
                            strengthen_group_score --;
                        }
                    }
                    if (marb_in_dir == black_marble) {
                        if (marb_in_back_dir == black_marble) {
                            lines_of_3--;
                        }
                    }
                }
            }
        }
    }
    features[0] = proximity_score_white - proximity_score_black; // proximity score difference
    features[1] = proximity_score_white/white_marbles - proximity_score_black/black_marbles; // proximity score difference
    features[2] = proximity_squared_score_white/white_marbles - proximity_squared_score_black/black_marbles; // normalized by number of marbles
    features[3] = cohesion_score;
    features[4] = break_strong_group_score;
    features[5] = strengthen_group_score;
    features[6] = lines_of_3;
    features[7] = color == WHITE ? (STARTING_MARBLES_BLACK - black_marbles) : (STARTING_MARBLES_WHITE - white_marbles); // difference in marbles
    features[8] = color == WHITE ? -(STARTING_MARBLES_WHITE - white_marbles) : -(STARTING_MARBLES_BLACK - black_marbles); // difference in marbles, but negative for black
}


void get_special_features(board* b, char color, double* features) {
    double white_marbles = 0;
    double black_marbles = 0;
    double proximity_score_white = 0;
    double proximity_score_black = 0;
    double proximity_squared_score_white = 0;
    double proximity_squared_score_black = 0;
    double cohesion_score = 0;
    double break_strong_group_score = 0;
    double strengthen_group_score = 0;
    double lines_of_3 = 0;
    for (int i = -RADIUS + 1; i < RADIUS; i++) {
        for (int j = -RADIUS + 1; j < RADIUS; j++) {
            char marb = get_marb_in_square(b, i, j);
            if (marb == white_marble) {
                white_marbles++;
                int dist = hex_distance(i, j, 0, 0);
                proximity_score_white += RADIUS - dist - 1;
                proximity_squared_score_white += dist * dist * dist;
                cohesion_score += get_no_neighbours(b, i, j, WHITE);
                for (enum directions d = 0; d < 6; d++) {
                    char marb_in_dir = get_marb_dir1(b, i, j, d);
                    char marb_in_back_dir = get_marb_dir1(b, i, j, get_backward_direction(d));
                    if (marb_in_dir == black_marble) {
                        if (marb_in_back_dir == black_marble) {
                            break_strong_group_score ++;
                        }
                        else if (marb_in_back_dir == white_marble) {
                            strengthen_group_score ++;
                        }
                    }
                    if (marb_in_dir == white_marble) {
                        if (marb_in_back_dir == white_marble) {
                            lines_of_3 += RADIUS - distance_of_line_from_square(i, j, d, 0, 0);
                        }
                    }
                }
            }
            else if (marb == black_marble) {
                black_marbles++;
                int dist = hex_distance(i, j, 0, 0);
                proximity_score_black += RADIUS - dist - 1;
                proximity_squared_score_black += dist * dist * dist;
                cohesion_score -= get_no_neighbours(b, i, j, BLACK);
                for (enum directions d = 0; d < 6; d++) {
                    char marb_in_dir = get_marb_dir1(b, i, j, d);
                    char marb_in_back_dir = get_marb_dir1(b, i, j, get_backward_direction(d));
                    if (marb_in_dir == white_marble) {
                        if (marb_in_back_dir == white_marble) {
                            break_strong_group_score -- ;
                        }
                        else if (marb_in_back_dir == black_marble) {
                            strengthen_group_score --;
                        }
                    }
                    if (marb_in_dir == black_marble) {
                        if (marb_in_back_dir == black_marble) {
                            lines_of_3 -= RADIUS - distance_of_line_from_square(i, j, d, 0, 0);
                        }
                    }
                }
            }
        }
    }
    features[0] = proximity_score_white - proximity_score_black; // proximity score difference
    features[1] = proximity_score_white/white_marbles - proximity_score_black/black_marbles; // proximity score difference
    features[2] = proximity_squared_score_white/white_marbles - proximity_squared_score_black/black_marbles; // normalized by number of marbles
    features[3] = cohesion_score;
    features[4] = break_strong_group_score;
    features[5] = strengthen_group_score;
    features[6] = lines_of_3;
    features[7] = (STARTING_MARBLES_BLACK - black_marbles) - (STARTING_MARBLES_WHITE - white_marbles); // difference in marbles
    features[8] = pow(2,proximity_score_white/white_marbles) * (STARTING_MARBLES_BLACK - black_marbles) -
                  pow(2,proximity_score_black/black_marbles) * (STARTING_MARBLES_WHITE - white_marbles);
}

double get_endgame_score(board *b, char color, double *weights) {
    double white_marbles = 0;
    double black_marbles = 0;
    double white_proximity_score = 0;
    double black_proximity_score = 0;
    double white_cohesion_score = 0;
    double black_cohesion_score = 0;
    for (int i = -RADIUS + 1; i < RADIUS; i++) {
        for (int j = -RADIUS + 1; j < RADIUS; j++) {
            char marb = get_marb_in_square(b, i, j);
            if (marb == white_marble) {
                white_marbles++;
                int dist = hex_distance(i, j, 0, 0);
                white_proximity_score += RADIUS - dist - 1;
                white_cohesion_score += get_no_neighbours(b, i, j, WHITE);
            }
            else if (marb == black_marble) {
                black_marbles++;
                int dist = hex_distance(i, j, 0, 0);
                black_proximity_score += RADIUS - dist - 1;
                black_cohesion_score += get_no_neighbours(b, i, j, BLACK);
            }
        }
    }
    white_proximity_score = white_proximity_score / (white_marbles);
    black_proximity_score = black_proximity_score / (black_marbles);
    white_cohesion_score = white_cohesion_score / (white_marbles);
    black_cohesion_score = black_cohesion_score / (black_marbles);
    double endgame_score_white = white_cohesion_score * weights[0] + 
                                 white_proximity_score * weights[1] +
                                    (STARTING_MARBLES_BLACK - black_marbles) * weights[2];
    double endgame_score_black = black_cohesion_score * weights[0] + 
                                 black_proximity_score * weights[1] +
                                    (STARTING_MARBLES_WHITE - white_marbles) * weights[2];
    double endgame_score = ABS(endgame_score_white - endgame_score_black);
    endgame_score *= endgame_score;
    //printf("endgame %lf\n", endgame_score);
    return MIN(endgame_score, 1.0);
}

/***************************************************************************************************************************/
// ReLU activation
static inline double relu(double x) {
    return x > 0.0 ? x : 0.0;
}

double sigmoid(double x) {
    // Sigmoid function to normalize output between 0 and 1
    return 1.0 / (1.0 + exp(-x));
}


// weights layout: 
// fc1_weights[HIDDEN_SIZE][INPUT_SIZE] (row-major)
// fc1_bias[HIDDEN_SIZE]
// fc2_weights[1][HIDDEN_SIZE] (just 8 weights)
// fc2_bias[1]
#define INPUT_SIZE FEATURE_COUNT
#define HIDDEN1_SIZE 16
#define HIDDEN2_SIZE 8

static inline double get_nn1_output(double *input, double *weights) {
    double hidden1[HIDDEN1_SIZE];
    double hidden2[HIDDEN2_SIZE];

    // fc1: input -> hidden1, no bias, tanh
    for (int j = 0; j < HIDDEN1_SIZE; j++) {
        double sum = 0.0;
        for (int i = 0; i < INPUT_SIZE; i++) {
            sum += input[i] * weights[j * INPUT_SIZE + i];
        }
        hidden1[j] = tanh(sum);
    }

    // fc2: hidden1 -> hidden2, no bias, tanh
    int offset = HIDDEN1_SIZE * INPUT_SIZE;
    for (int j = 0; j < HIDDEN2_SIZE; j++) {
        double sum = 0.0;
        for (int i = 0; i < HIDDEN1_SIZE; i++) {
            sum += hidden1[i] * weights[offset + j * HIDDEN1_SIZE + i];
        }
        hidden2[j] = tanh(sum);
    }

    // fc3: hidden2 -> output, no bias
    offset += HIDDEN2_SIZE * HIDDEN1_SIZE;
    double output = 0.0;
    for (int j = 0; j < HIDDEN2_SIZE; j++) {
        output += hidden2[j] * weights[offset + j];
    }

    return tanh(output);
}

float get_nn2_output(double *input, double *weights) {
    double output = 0.0f;

    for (int i = 0; i < INPUT_SIZE; i++) {
        output += input[i] * weights[i] + (input[i] * (2 * ABS(input[i]) + 1)) * weights[i + INPUT_SIZE];
    }
    return output;
}

float get_nn3_output(double *input, double *weights) {
    // like nn1 but with relu activations
    double hidden1[HIDDEN1_SIZE];
    double hidden2[HIDDEN2_SIZE];
    // fc1: input -> hidden1, no bias, relu
    for (int j = 0; j < HIDDEN1_SIZE; j++) {
        double sum = 0.0;
        for (int i = 0; i < INPUT_SIZE; i++) {
            sum += input[i] * weights[j * INPUT_SIZE + i];
        }
        hidden1[j] = relu(sum);
    }
    // fc2: hidden1 -> hidden2, no bias, relu
    int offset = HIDDEN1_SIZE * INPUT_SIZE;
    for (int j = 0; j < HIDDEN2_SIZE; j++) {
        double sum = 0.0;
        for (int i = 0; i < HIDDEN1_SIZE; i++) {
            sum += hidden1[i] * weights[offset + j * HIDDEN1_SIZE + i];
        }
        hidden2[j] = relu(sum);
    }
    // fc3: hidden2 -> output, no bias
    offset += HIDDEN2_SIZE * HIDDEN1_SIZE;
    double output = 0.0;
    for (int j = 0; j < HIDDEN2_SIZE; j++) {
        output += hidden2[j] * weights[offset + j];
    }
    return tanh(output);
}

double evaluate_nn1(board *b, char color) {
    double features[FEATURE_COUNT];
    get_special_features(b, color, features);
    double MAX_ABS_VALUES[FEATURE_COUNT] = {34. ,2., 64, 47., 18., 18.,  80.,  6.,  20.};    // Normalize features
    for (int i = 0; i < FEATURE_COUNT; i++) {
        features[i] /= MAX_ABS_VALUES[i];
    }
    return get_nn1_output(features, self_play_weights1);
}


double evaluate_nn2(board *b, char color) {
    double features[FEATURE_COUNT];
    get_special_features(b, color, features);
    double MAX_ABS_VALUES[FEATURE_COUNT] = {34. ,2., 64, 47., 18., 18.,  80.,  6.,  20.};    // Normalize features
    for (int i = 0; i < FEATURE_COUNT; i++) {
        features[i] /= MAX_ABS_VALUES[i];
    }
    return (get_nn1_output(features, self_play_weights2));
}

double evaluate_nn3(board *b, char color) {
    double features[FEATURE_COUNT];
    get_special_features(b, color, features);
    double MAX_ABS_VALUES[FEATURE_COUNT] = {34. ,2., 64, 47., 18., 18.,  80.,  6.,  20};    // Normalize features
    for (int i = 0; i < FEATURE_COUNT; i++) {
        features[i] /= MAX_ABS_VALUES[i];
    }
    return (get_nn2_output(features, self_play_weights3));
}

double evaluate_nn4(board *b, char color) {
    double features[FEATURE_COUNT];
    get_special_features(b, color, features);
    double MAX_ABS_VALUES[FEATURE_COUNT] = {34. ,2., 64, 47., 18., 18.,  80.,  6.,  20.};    // Normalize features
    for (int i = 0; i < FEATURE_COUNT; i++) {
        features[i] /= MAX_ABS_VALUES[i];
    }
    return (get_nn2_output(features, self_play_weights4));
}

/******************************************************************************************************************************************************************************* */

/* Evaluation function: returns score from white's perspective */
double evaluate(board* b, char color) {
    //print_board(b);
    
    if (evaluation_function_number == 4) {
        return evaluate_nn1(b, color);
    }
    else if (evaluation_function_number == 5) {
        return evaluate_nn2(b, color);
    }
    else if (evaluation_function_number == 6) {
        return evaluate_nn3(b, color);
    }
    else if (evaluation_function_number == 7) {
        return evaluate_nn4(b, color);
    }
}



double evaluate_features(double *features, char color) {
    double MAX_ABS_VALUES[FEATURE_COUNT] = {34. ,2., 64., 47., 18., 18.,  80.,  6.,  20.};    // Normalize features
    for (int i = 0; i < FEATURE_COUNT; i++) {
        features[i] /= MAX_ABS_VALUES[i];
    }
    double output = get_nn1_output(features, self_play_weights1);
    return output;
}

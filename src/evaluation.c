#include "../include/evaluation.h"


#include <stdlib.h>
#include <math.h>

#define STARTING_MARBLES_WHITE 14
#define STARTING_MARBLES_BLACK 14

char evaluation_function_number = 0;


#define POLY(x, a1, a2, a3, a4) \
    (((x) >= 0) * (a1) + (a2) * (x) + ((x) >= 0) * (a3) * (x) * (x) + (a4) * (x) * (x) * (x))
    


double weights[7][4];


/*
1.194446206092834473e+00 8.374558687210083008e-01 1.539749056100845337e-01
1.675892025232315063e-01 -1.717536449432373047e-01 -2.826795578002929688e-01
8.011112809181213379e-01 -1.251622140407562256e-01 -2.794158160686492920e-01
4.773312211036682129e-01 -1.776414215564727783e-01 -2.470646202564239502e-01
3.367320209772535421e-42 8.113518108440690841e-43 -1.626907517081112619e-42
3.721568286418914795e-01 7.216846346855163574e-01 -2.553943395614624023e-01
2.146258950233459473e-01 3.442228436470031738e-01 6.158503890037536621e-01


*/

double poly_coefficients_1[7][4] = {
    {1.194446206092834473e+00, 8.374558687210083008e-01, 1.539749056100845337e-01, 0}, 
    {1.675892025232315063e-01, -1.717536449432373047e-01, -2.826795578002929688e-01, 0},
    {8.011112809181213379e-01, -1.251622140407562256e-01, -2.794158160686492920e-01, 0},
    {4.773312211036682129e-01, -1.776414215564727783e-01, -2.470646202564239502e-01, 0},
    {3.367320209772535421e-42, 8.113518108440690841e-43, -1.626907517081112619e-42, 0},
    {3.721568286418914795e-01, 7.216846346855163574e-01, -2.553943395614624023e-01, 0},
    {2.146258950233459473e-01, 3.442228436470031738e-01, 6.158503890037536621e-01, 0}
    };

double poly_coefficients_2[7][4] = {
    {0, 1.327386021614074707e+00, -6.866407394409179688e-01, 0}, 
    {0, -1.408225595951080322e-01, -5.104897618293762207e-01, 0},
    {0, 2.219095975160598755e-01, -2.922518849372863770e-01, 0},
    {0, 1.886795461177825928e-01, -3.799577653408050537e-01, 0},
    {0, 0, 0, 0},
    {0, 6.940866708755493164e-01, -1.376750916242599487e-01, 0},
    {0, 2.223298996686935425e-01, 7.425394654273986816e-01, 0}
    };


/* 
-4.918685182929039001e-03
1.377974003553390503e-01
4.569528698921203613e-01
3.710978329181671143e-01
-1.936314218004032229e-41
-1.455489825457334518e-02
1.163885354995727539e+00

*/
double modus_weights[7] = {
    -4.918685182929039001e-03,
    1.377974003553390503e-01,
    4.569528698921203613e-01,
    3.710978329181671143e-01,
    -1.936314218004032229e-41,
    -1.455489825457334518e-02,
    1.163885354995727539e+00
};


double weights_table_1[10][7] = {
    {3,2,6,1.8,2,0,0},
    {3.3,2,6,1.8,2,35,35},
    {2.9,2,15,3,2,10,10},
    {2.9,2,15,3,2,15,15},
    {2.8,2.3,25,3,2,15,15},
    {2.8,2.1,25,3,3,25,25},
    {2.7,2.3,25,3,4,30,30},
    {2.4,2.3,25,3,6,35,35},
    {2.2,2.3,25,3,8,40,40},
    {1.3,2.3,25,3,15,85,85},
    };  



// Clamp helper
static inline double clamp(double x, double low, double high) {
    if (x < low) return low;
    if (x > high) return high;
    return x;
}

// Compute early, mid, late weights for a given modus in [0,1]
void compute_phase_weights(double modus, double* w_early, double* w_mid, double* w_late) {
    *w_early = clamp(1.0 - 2.0 * modus, 0.0, 1.0);
    *w_mid   = clamp(1.0 - 2.0 * fabs(modus - 0.5), 0.0, 1.0);
    *w_late  = clamp(2.0 * modus - 1.0, 0.0, 1.0);
}


char get_modus(double center_proximity, double cohesion) {
    if (center_proximity < 0)
        return 1;
    else if (center_proximity < 5)
        return 2;
    else {
        if (cohesion < 4)
            return 3;
        else if (cohesion < 10)
            return 4;
        else if (cohesion < 16)
            return 5;
        else if (cohesion < 22)
            return 6;
        else if (cohesion < 28)
            return 7;
        else if (cohesion < 34)
            return 8;
        else if (cohesion < 40)
            return 9;
        else {
            return 10;
        }
    }
}

double evaluate_original(board* b, char color) {
    /* We use these parameters to evaluate the board:
        * 1. Center proximity
        * 2. Cohesion
        * 3. Break strong group
        * 4. Strengthen group
        * 5. Opponent's marbles removed
        * 6. Self marbles removed (negative)
        */
    
    double score = 0;

    /* Now we'll check the center proximity (we use hex_distance for distance) */
    int white_marbles = 0;
    int black_marbles = 0;
    double proximity_score = 0;
    double cohesion_score = 0;
    double break_strong_group_score = 0;
    double strengthen_group_score = 0;
    double immediate_marb_capturing_danger_score = 0;
    for (int i = -RADIUS + 1; i < RADIUS; i++) {
        for (int j = -RADIUS + 1; j < RADIUS; j++) {
            char marb = get_marb_in_square(b, i, j);
            if (marb == white_marble) {
                white_marbles++;
                int dist = hex_distance(i, j, 0, 0);
                proximity_score += RADIUS - dist - 1;
                cohesion_score += get_no_neighbours(b, i, j, WHITE);
                for (enum directions d = 0; d < 6; d++) {
                    if (get_marb_in_direction(b, i, j, d, 1) == black_marble) {
                        if (get_marb_in_direction(b, i, j, get_backward_direction(d), 1) == black_marble) {
                            break_strong_group_score ++;
                        }
                        else if (get_marb_in_direction(b, i, j, get_backward_direction(d), 1) == white_marble) {
                            strengthen_group_score ++;
                        }
                    }
                }
            }
            else if (marb == black_marble) {
                black_marbles++;
                int dist = hex_distance(i, j, 0, 0);
                proximity_score -= RADIUS - dist - 1;
                cohesion_score -= get_no_neighbours(b, i, j, BLACK);
                for (enum directions d = 0; d < 6; d++) {
                    if (get_marb_in_direction(b, i, j, d, 1) == white_marble) {
                        if (get_marb_in_direction(b, i, j, get_backward_direction(d), 1) == white_marble) {
                            break_strong_group_score -- ;
                        }
                        else if (get_marb_in_direction(b, i, j, get_backward_direction(d), 1) == black_marble) {
                            strengthen_group_score --;
                        }
                    }
                }
            }
        }
    }
    if (evaluation_function_number == 2) {
        double MAX_ABS_VALUES[7] = {26., 52., 18., 21.,  0.,  6.,  6.};
        score += POLY(proximity_score / MAX_ABS_VALUES[0], weights[0][0], weights[0][1], weights[0][2], weights[0][3]);
        score += POLY(cohesion_score / MAX_ABS_VALUES[1], weights[1][0], weights[1][1], weights[1][2], weights[1][3]);
        score += POLY(break_strong_group_score / MAX_ABS_VALUES[2], weights[2][0], weights[2][1], weights[2][2], weights[2][3]);
        score += POLY(strengthen_group_score / MAX_ABS_VALUES[3], weights[3][0], weights[3][1], weights[3][2], weights[3][3]);
        score += POLY((STARTING_MARBLES_BLACK - black_marbles) / MAX_ABS_VALUES[5], weights[5][0], weights[5][1], weights[5][2], weights[5][3]);
        score -= POLY((STARTING_MARBLES_WHITE - white_marbles) / MAX_ABS_VALUES[6], weights[6][0], weights[6][1], weights[6][2], weights[6][3]);        
    }
    else if (evaluation_function_number == 1) {
        double MAX_ABS_VALUES[7] = {26., 52., 18., 21.,  0.,  6.,  6.};
        double modus = modus_weights[1] * abs(proximity_score) / MAX_ABS_VALUES[0] +
                       modus_weights[2] * abs(cohesion_score) / MAX_ABS_VALUES[1] +
                       modus_weights[3] * abs(break_strong_group_score) / MAX_ABS_VALUES[2] +
                       modus_weights[4] * abs(strengthen_group_score) / MAX_ABS_VALUES[3] +
                       modus_weights[5] * abs(STARTING_MARBLES_BLACK - black_marbles) / MAX_ABS_VALUES[6] -
                       modus_weights[6] * abs(STARTING_MARBLES_WHITE - white_marbles) / MAX_ABS_VALUES[5];
        if (modus < 0) modus = 0;
        else if (modus > 1) modus = 1;
        double w_early, w_mid, w_late;
        compute_phase_weights(modus, &w_early, &w_mid, &w_late);
        score += w_early * poly_coefficients_1[0][0] * (proximity_score / MAX_ABS_VALUES[0]) +
                 w_mid * poly_coefficients_1[1][0] * (proximity_score / MAX_ABS_VALUES[0]) +
                    w_late * poly_coefficients_1[2][0] * (proximity_score / MAX_ABS_VALUES[0]);
        score += w_early * poly_coefficients_1[0][1] * (cohesion_score / MAX_ABS_VALUES[1]) +
                    w_mid * poly_coefficients_1[1][1] * (cohesion_score / MAX_ABS_VALUES[1]) +
                        w_late * poly_coefficients_1[2][1] * (cohesion_score / MAX_ABS_VALUES[1]);
        score += w_early * poly_coefficients_1[0][2] * (break_strong_group_score / MAX_ABS_VALUES[2]) +
                    w_mid * poly_coefficients_1[1][2] * (break_strong_group_score / MAX_ABS_VALUES[2]) +
                        w_late * poly_coefficients_1[2][2] * (break_strong_group_score / MAX_ABS_VALUES[2]);
        score += w_early * poly_coefficients_1[0][3] * (strengthen_group_score / MAX_ABS_VALUES[3]) +
                    w_mid * poly_coefficients_1[1][3] * (strengthen_group_score / MAX_ABS_VALUES[3]) +
                        w_late * poly_coefficients_1[2][3] * (strengthen_group_score / MAX_ABS_VALUES[3]);
        score += w_early * poly_coefficients_1[0][5] * (STARTING_MARBLES_BLACK - black_marbles) / MAX_ABS_VALUES[5] +
                    w_mid * poly_coefficients_1[1][5] * (STARTING_MARBLES_BLACK - black_marbles) / MAX_ABS_VALUES[5] +
                        w_late * poly_coefficients_1[2][5] * (STARTING_MARBLES_BLACK - black_marbles) / MAX_ABS_VALUES[5];
        score -= w_early * poly_coefficients_1[0][6] * (STARTING_MARBLES_WHITE - white_marbles) / MAX_ABS_VALUES[6] +
                    w_mid * poly_coefficients_1[1][6] * (STARTING_MARBLES_WHITE - white_marbles) / MAX_ABS_VALUES[6] +
                        w_late * poly_coefficients_1[2][6] * (STARTING_MARBLES_WHITE - white_marbles) / MAX_ABS_VALUES[6];

    }
    else {
        score = 0;
        char modus = color == WHITE ? get_modus(proximity_score, cohesion_score) : get_modus(-proximity_score, -cohesion_score);
        score += (proximity_score) * weights_table_1[modus - 1][0];
        score += (cohesion_score) * weights_table_1[modus - 1][1];
        score += (break_strong_group_score) * weights_table_1[modus - 1][2];
        score += (strengthen_group_score) * weights_table_1[modus - 1][3];
        score += (immediate_marb_capturing_danger_score) * weights_table_1[modus - 1][4];
        if (color == WHITE) {
            score += (STARTING_MARBLES_BLACK - black_marbles) * weights_table_1[modus - 1][5] -
                     (STARTING_MARBLES_WHITE - white_marbles) * weights_table_1[modus - 1][6];
        }
        else {
            score += (STARTING_MARBLES_BLACK - black_marbles) * weights_table_1[modus - 1][5] -
                     (STARTING_MARBLES_WHITE - white_marbles) * weights_table_1[modus - 1][6];
        }
    }

    
    /*
    printf(color == WHITE ? "WHITE: " : "BLACK: \n");
    print_board(b);
    printf("Proximity: %f, Cohesion: %f, Break: %f, Strengthen: %f, Opponent's marbles removed: %f\n",
                (white_proximity-black_proximity) * weights[0],
                (white_cohesion-black_cohesion) * weights[1],
                (white_break-black_break) * weights[2],
                (white_strengthen-black_strengthen) * weights[3],
                color == WHITE ? (STARTING_MARBLES_BLACK - black_marbles) * weights[4] : 
                -(STARTING_MARBLES_WHITE - white_marbles) * weights[4]);
    */
    return score / 100;
}

void get_features(board* b, char color, double* features) {
    double white_marbles = 0;
    double black_marbles = 0;
    double proximity_score = 0;
    double cohesion_score = 0;
    double break_strong_group_score = 0;
    double strengthen_group_score = 0;
    double immediate_marb_capturing_danger_score = 0;
    for (int i = -RADIUS + 1; i < RADIUS; i++) {
        for (int j = -RADIUS + 1; j < RADIUS; j++) {
            char marb = get_marb_in_square(b, i, j);
            if (marb == white_marble) {
                white_marbles++;
                int dist = hex_distance(i, j, 0, 0);
                proximity_score += RADIUS - dist - 1;
                cohesion_score += get_no_neighbours(b, i, j, WHITE);
                for (enum directions d = 0; d < 6; d++) {
                    if (get_marb_in_direction(b, i, j, d, 1) == black_marble) {
                        if (get_marb_in_direction(b, i, j, get_backward_direction(d), 1) == black_marble) {
                            break_strong_group_score ++;
                        }
                        else if (get_marb_in_direction(b, i, j, get_backward_direction(d), 1) == white_marble) {
                            strengthen_group_score ++;
                        }
                    }
                }
            }
            else if (marb == black_marble) {
                black_marbles++;
                int dist = hex_distance(i, j, 0, 0);
                proximity_score -= RADIUS - dist - 1;
                cohesion_score -= get_no_neighbours(b, i, j, BLACK);
                for (enum directions d = 0; d < 6; d++) {
                    if (get_marb_in_direction(b, i, j, d, 1) == white_marble) {
                        if (get_marb_in_direction(b, i, j, get_backward_direction(d), 1) == white_marble) {
                            break_strong_group_score -- ;
                        }
                        else if (get_marb_in_direction(b, i, j, get_backward_direction(d), 1) == black_marble) {
                            strengthen_group_score --;
                        }
                    }
                }
            }
        }
    }
    features[0] = proximity_score;
    features[1] = cohesion_score;
    features[2] = break_strong_group_score;
    features[3] = strengthen_group_score;
    features[4] = immediate_marb_capturing_danger_score;
    if (color == WHITE) {
        features[5] = (STARTING_MARBLES_BLACK - black_marbles);
        features[6] = -(STARTING_MARBLES_WHITE - white_marbles);
    }
    else {
        features[5] = (STARTING_MARBLES_BLACK - black_marbles);
        features[6] = -(STARTING_MARBLES_WHITE - white_marbles);
    }
}
  
double lightweight_evaluate(board* b, char color) {
    /* Use only cohesion and center proximity */
    double score = 0;
    int white_marbles = 0;
    int black_marbles = 0;
    double proximity_score = 0;
    double cohesion_score = 0;
    for (int i = -RADIUS + 1; i < RADIUS; i++) {
        for (int j = -RADIUS + 1; j < RADIUS; j++) {
            char marb = get_marb_in_square(b, i, j);
            if (marb == white_marble) {
                int dist = hex_distance(i, j, 0, 0);
                proximity_score += RADIUS - dist - 1;
                cohesion_score += get_no_neighbours(b, i, j, WHITE);
                white_marbles++;
            }
            else if (marb == black_marble) {
                int dist = hex_distance(i, j, 0, 0);
                proximity_score -= RADIUS - dist - 1;
                cohesion_score -= get_no_neighbours(b, i, j, BLACK);
                black_marbles++;
            }
        }
    }
    char modus = color == WHITE ? get_modus(proximity_score, cohesion_score) : get_modus(-proximity_score, -cohesion_score);
    score += (proximity_score) * weights_table_1[modus - 1][0];
    score += (cohesion_score) * weights_table_1[modus - 1][1];
    if (color == WHITE) {
        score += (STARTING_MARBLES_BLACK - black_marbles) * weights_table_1[modus - 1][5] -
                 (STARTING_MARBLES_WHITE - white_marbles) * weights_table_1[modus - 1][6];
    }
    else {
        score += (STARTING_MARBLES_BLACK - black_marbles) * weights_table_1[modus - 1][6] -
                 (STARTING_MARBLES_WHITE - white_marbles) * weights_table_1[modus - 1][5];
    }
    /*
    if (score < -200 || score > 200) {
        printf("score: %f\n", score);
    }
    */
    return score / 100;
}



/* Evaluation function: returns score from white's perspective */
double evaluate(board* b, char color) {
    //print_board(b);
    
    if (evaluation_function_number == 4)
        return lightweight_evaluate(b, color);
    else if (evaluation_function_number == 1 || evaluation_function_number == 2) {
        return evaluate_original(b, color);
    }
    else {
        return evaluate_original(b, color) + evaluate_original(b, color == WHITE ? BLACK : WHITE);
    }
}


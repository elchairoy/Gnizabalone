#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>


#include "evaluation.h"
#include "uci.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define NUM_PHASES 4

extern char eval_function_by_color[2];

double evaluate_position(double features[NUM_FEATURES], double weights[7][NUM_PHASES]) {
    double score = 0;
    double x = features[7]; // x is features[7]
    double x_terms[4] = {1, x, x * x, x * x * x};
    for (int i = 0; i < 7; i++) { // Only 7 features have weights
        double poly = 0;
        for (int j = 0; j < NUM_PHASES; j++) {
            poly += weights[i][j] * x_terms[j];
        }
        score += poly * features[i];
    }
    return score;
}

void normalize_dataset(Dataset *dataset) {
    double mean_f[NUM_FEATURES] = {0}, std_f[NUM_FEATURES] = {0};
    int n = dataset->num_positions;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < NUM_FEATURES; j++) {
            mean_f[j] += dataset->positions[i].features[j];
        }
    }
    for (int j = 0; j < NUM_FEATURES; j++) mean_f[j] /= n;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < NUM_FEATURES; j++) {
            std_f[j] += pow(dataset->positions[i].features[j] - mean_f[j], 2);
        }
    }
    for (int j = 0; j < NUM_FEATURES; j++) std_f[j] = sqrt(std_f[j] / n + 1e-8);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < NUM_FEATURES; j++) {
            dataset->positions[i].features[j] = (dataset->positions[i].features[j] - mean_f[j]) / std_f[j];
        }
    }
}

void save_dataset_to_csv(Dataset *dataset, const char *filename) {
    FILE *fp = fopen(filename, "a");
    if (!fp) {
        printf("Error opening file %s\n", filename);
        return;
    }
    fprintf(fp, "f1,f2,f3,f4,f5,f6,f7,target\n"); // 8 features
    for (int i = 0; i < dataset->num_positions; i++) {
        Position *pos = &dataset->positions[i];
        for (int j = 0; j < NUM_FEATURES; j++) {
            fprintf(fp, "%f,", pos->features[j]);
        }
        fprintf(fp, "%f\n", pos->target);
    }
    fclose(fp);
}

void collect_positions(Dataset *dataset, HashTable *ht, char depth, const char *filename) {
    dataset->num_positions = 0;

    for (int game = 0; dataset->num_positions < MAX_POSITIONS-200; game++) {
        char temp = eval_function_by_color[0];
        eval_function_by_color[0] = eval_function_by_color[1];
        eval_function_by_color[1] = temp;
        printf("Game %d\n", game);
        double score = simulate(ht, depth, dataset);
        ht_clear(ht);
        

    }
    save_dataset_to_csv(dataset, filename);
}

void load_weights(double weights[7][NUM_PHASES], const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Error opening file %s\n", filename);
        return;
    }
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < NUM_PHASES; j++) {
            fscanf(fp, "%lf", &weights[i][j]);
        }
    }
    fclose(fp);
}

int main_fine_tune(HashTable *ht) {
    char depth = 2;

    // Generate dataset
    Dataset dataset;
    for (int i = 0; i < 10; i++) 
        collect_positions(&dataset, ht, depth, "dataset.csv");


    
    return 0;
}

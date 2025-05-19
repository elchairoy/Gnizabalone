#ifndef AFD0BF27_C006_46B4_89E6_171B4DC00123
#define AFD0BF27_C006_46B4_89E6_171B4DC00123

#define MAX_POSITIONS 50000
#define NUM_FEATURES 7


int main_fine_tune(HashTable *ht);

typedef struct {
    double features[NUM_FEATURES];
    double target;
} Position;

typedef struct {
    Position positions[MAX_POSITIONS];
    int num_positions;
} Dataset;

#endif /* AFD0BF27_C006_46B4_89E6_171B4DC00123 */
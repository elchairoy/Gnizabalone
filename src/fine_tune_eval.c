#include "evaluation.h"
#include "uci.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define DEPTH 4
#define NO_GAMES 4

extern double self_play_weights1[WEIGHT_COUNT];
extern double self_play_weights2[WEIGHT_COUNT];
extern double self_play_weights5[WEIGHT_COUNT];
extern char eval_function_by_color[2];

// TRAINING INDEXES 0, 1, 2, 4, 5, 6, 10, 11, 20, 21, 22, 26, 27 only

// Assuming these are defined elsewhere in your code
#define POPULATION_SIZE 10
#define ELITISM_COUNT 4 // Number of top individuals to carry over unchanged

#define ABS(x) ((x) < 0 ? -(x) : (x))

typedef struct {
    double weights[WEIGHT_COUNT];
    double fitness;
} Individual;

// Function to compare individuals by fitness for sorting
int compare_fitness(const void *a, const void *b) {
    Individual *ind1 = (Individual *)a;
    Individual *ind2 = (Individual *)b;
    // Sort in descending order (highest fitness first)
    if (ind1->fitness > ind2->fitness) return -1;
    if (ind1->fitness < ind2->fitness) return 1;
    return 0;
}

// Function to initialize the first population
void initialize_population(Individual *population) {
    // Seed the random number generator
    srand(time(NULL));
    
    // Fill the population with a known good set of weights
    // This assumes self_play_weights1 is a good starting point
    for (int i = 0; i < POPULATION_SIZE; i++) {
        memcpy(population[i].weights, self_play_weights1, sizeof(double) * WEIGHT_COUNT);
        
        // Add random variation to all individuals (except the first one, which is the baseline)
        if (i > 0) {
            for (int j = 0; j < WEIGHT_COUNT; j++) {
                // Perturb each weight by up to +/- 10%
                population[i].weights[j] *= ((double)rand() / RAND_MAX) * 0.2 + 0.9;
            }
        }
        population[i].fitness = 0.0; // Initialize fitness to zero
    }

    // normalize the weights of all individuals to sum up to one.
    // normalize every part in itself - 0 to 3, 4 to 20, 20 to WEIGHT_COUNT
    for (int i = 0; i < POPULATION_SIZE; i++) {
        double sum = 0.0;
        for (int j = 0; j < WEIGHT_COUNT; j++) {
            sum += ABS(population[i].weights[j]);
        }
        if (sum != 0.0) {
            for (int j = 0; j < WEIGHT_COUNT; j++) {
                population[i].weights[j] /= sum;
            }
        }
    }
}

// Function to evaluate the fitness of each individual in the population
void evaluate_population_fitness(HashTable *ht, Individual *population) {
    for (int i = 0; i < POPULATION_SIZE; i++) {
        printf("Evaluating individual %d...\n", i);
        // Set player 1's weights to the current individual's weights
        memcpy(self_play_weights1, population[i].weights, sizeof(double) * WEIGHT_COUNT);
        
        // Assuming player 2's weights are static for a fair comparison.
        // If you want to play against a different opponent, change self_play_weights2 here.
        
        // Simulate games and get a score
        // We'll use your original evaluation functions (5 and 4) for this comparison
        //double score = simulate_number_of_games(ht, 2, 2);
        double score = simulate_number_of_games(ht, DEPTH, NO_GAMES / 2);
        if (score > 0) {
            score += simulate_number_of_games(ht, DEPTH, NO_GAMES);
            population[i].fitness = score;
        }
        else {
            population[i].fitness = -1;
        }
        
        // Clear the hash table after each evaluation to ensure fair comparisons
        ht_clear(ht);
    }
}

// Function to create the next generation using selection, crossover, and mutation
void create_next_generation(Individual *old_population, Individual *new_population) {
    // 1. Sort the old population by fitness (fittest first)
    qsort(old_population, POPULATION_SIZE, sizeof(Individual), compare_fitness);

    // 2. Elitism: Copy the fittest individuals directly to the new generation
    for (int i = 0; i < ELITISM_COUNT; i++) {
        memcpy(&new_population[i], &old_population[i], sizeof(Individual));
    }

    // 3. Create new individuals via crossover and mutation
    for (int i = ELITISM_COUNT; i < POPULATION_SIZE; i++) {
        // Tournament selection: select two parents from the old population
        // The more fit an individual, the more likely it is to be a parent
        int parent1_idx = rand() % POPULATION_SIZE;
        int parent2_idx = rand() % POPULATION_SIZE;

        // Crossover: create a child from the two selected parents
        for (int j = 0; j < WEIGHT_COUNT; j++) {
            if (rand() % 2 == 0) {
                new_population[i].weights[j] = old_population[parent1_idx].weights[j];
            } else {
                new_population[i].weights[j] = old_population[parent2_idx].weights[j];
            }
        }
        
        // Mutation: apply a small random change to the new child's weights, both in multiplication and in addition
        for (int j = 0; j < WEIGHT_COUNT; j++) {
            new_population[i].weights[j] *= ((double)rand() / RAND_MAX) * 0.2 + 0.9; // Random multiplication factor between 0.95 and 1.05
            new_population[i].weights[j] += ((double)rand() / RAND_MAX) * 0.1 - 0.05; // Random change between -0.005 and +0.005
        }
        new_population[i].fitness = 0.0; // Reset fitness for the new generation
    }
}

// The main genetic algorithm optimization function
void genetic_algorithm_optimization(HashTable *ht) {
    Individual current_population[POPULATION_SIZE];
    Individual next_population[POPULATION_SIZE];
    
    // Initialize the first population
    initialize_population(current_population);
    
    for (int generation = 0; generation < 1000; generation++) {
        printf("--- Starting Generation %d ---\n", generation);

        // 1. Evaluate the fitness of the current population
        evaluate_population_fitness(ht, current_population);
        
        // 2. Print the best fitness score and weights for this generation
        qsort(current_population, POPULATION_SIZE, sizeof(Individual), compare_fitness);
        printf("Generation %d best fitness: %lf\n", generation, current_population[0].fitness);
        //memcpy(self_play_weights2, current_population[0].weights, sizeof(double) * WEIGHT_COUNT);
        printf("Best weights: ");
        for (int j = 0; j < WEIGHT_COUNT; j++) {
            printf("%lf ", current_population[0].weights[j]);
        }
        printf("\n");
        
        // 3. Create the next generation
        create_next_generation(current_population, next_population);
        
        // 4. Swap the populations for the next iteration
        memcpy(current_population, next_population, sizeof(Individual) * POPULATION_SIZE);
    }
    
    // Final evaluation and result display
    evaluate_population_fitness(ht, current_population);
    qsort(current_population, POPULATION_SIZE, sizeof(Individual), compare_fitness);
    
    printf("\n--- Optimization Complete ---\n");
    printf("Final best weights found:\n");
    for (int j = 0; j < WEIGHT_COUNT; j++) {
        printf("%lf ", current_population[0].weights[j]);
    }
    printf("\nFinal best fitness: %lf\n", current_population[0].fitness);
    
    // You can now copy these final best weights to self_play_weights1 for use
    memcpy(self_play_weights2, current_population[0].weights, sizeof(double) * WEIGHT_COUNT);
}

int main_fine_tune(HashTable *ht) {
    eval_function_by_color[0] = 4;
    eval_function_by_color[1] = 6;
    genetic_algorithm_optimization(ht);
}
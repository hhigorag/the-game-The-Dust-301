#include "core/math/rng.h"
#include <stdlib.h>

static uint32_t g_seed = 0;

void RNG_Seed(uint32_t seed) {
    g_seed = seed;
    srand(seed);
}

int RNG_Rand(void) {
    return rand();
}

int RNG_RandInt(int max) {
    if (max <= 0) return 0;
    return rand() % max;
}

int RNG_RandRange(int min, int max) {
    if (min >= max) return min;
    return min + (rand() % (max - min + 1));
}

float RNG_RandFloat(void) {
    return (float)rand() / (float)RAND_MAX;
}

float RNG_RandFloatRange(float min, float max) {
    if (min >= max) return min;
    return min + RNG_RandFloat() * (max - min);
}

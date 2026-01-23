#ifndef RNG_H
#define RNG_H

#include <stdint.h>

// Inicializa o RNG com uma seed
void RNG_Seed(uint32_t seed);

// Retorna um número aleatório entre 0 e RAND_MAX
int RNG_Rand(void);

// Retorna um número aleatório entre 0 e max (exclusive)
int RNG_RandInt(int max);

// Retorna um número aleatório entre min e max (inclusive)
int RNG_RandRange(int min, int max);

// Retorna um float aleatório entre 0.0 e 1.0
float RNG_RandFloat(void);

// Retorna um float aleatório entre min e max
float RNG_RandFloatRange(float min, float max);

#endif // RNG_H

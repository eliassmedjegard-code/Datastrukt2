#pragma once
#include "algo.h"

// Alla generatorer returnerar en NY vector vid varje anrop. Det är
// viktigt eftersom mätningarna ska göras på fresh data - om vi
// återanvände en redan sorterad array skulle vi mäta "best case"
// istället för det vi tror oss mäta.

// Slumpvärden i [0, n] - rimlig spridning utan för många dubbletter.
vec_t random_gen(int n);

// 0, 1, 2, ..., n-1. Best case för insertion sort, worst case för
// naiv quicksort.
vec_t increasing_gen(int n);

// n-1, ..., 1, 0. Worst case för insertion sort (max antal shifts)
// och worst case för naiv quicksort.
vec_t decreasing_gen(int n);

// Alla element är samma värde. Trasar sönder Lomuto-quicksort även med
// median-of-three eftersom pivot-valet inte hjälper när alla är lika.
vec_t constant_gen(int n);

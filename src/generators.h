#pragma once
#include <vector>
#include <random>
#include <numeric>
#include "algo.h"

// Alla generatorer returnerar en NY vector varje anrop. Det är viktigt
// eftersom mätningarna ska göras på fresh data - om vi återanvänder
// en redan sorterad array skulle vi mäta "best case" istället för
// det vi tror att vi mäter.

// =====================================================================
// SLUMPDATA
// =====================================================================
// thread_local statisk RNG: vi seedar EN gång och får sedan en stadig
// ström av slumpvärden. Att återanvända generatorn över anrop är
// snabbare än att seeda om varje gång, och vi får ändå ny data per
// anrop eftersom RNG:n har internt tillstånd.
inline vec_t random_gen(int n) {
    static thread_local std::mt19937 rng{std::random_device{}()};
    // Värden i intervallet [0, n] - rimlig spridning för att undvika
    // alltför många dubbletter (relevant för Lomuto-quicksort).
    std::uniform_int_distribution<int> dist(0, n);
    vec_t v(n);
    for (auto& x : v) x = dist(rng);
    return v;
}

// =====================================================================
// MONOTONT STIGANDE
// =====================================================================
// 0, 1, 2, ..., n-1
// Best case för insertion sort (O(N) - inga shiftar behövs).
// Worst case för naiv quicksort (pivoten är alltid max).
inline vec_t increasing_gen(int n) {
    vec_t v(n);
    std::iota(v.begin(), v.end(), 0);
    return v;
}

// =====================================================================
// MONOTONT FALLANDE
// =====================================================================
// n-1, n-2, ..., 1, 0
// Worst case för insertion sort - varje element måste shiftas hela
// vägen från slutet till början. Också worst case för naiv quicksort.
inline vec_t decreasing_gen(int n) {
    vec_t v(n);
    std::iota(v.rbegin(), v.rend(), 0);
    return v;
}

// =====================================================================
// KONSTANT
// =====================================================================
// Alla element är samma. Intressant kontroll - för Lomuto-quicksort
// blir detta worst case även med median-of-three, eftersom pivoten
// inte hjälper när alla element redan är lika.
inline vec_t constant_gen(int n) {
    return vec_t(n, 42);
}

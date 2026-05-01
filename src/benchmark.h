#pragma once
#include <chrono>
#include <vector>
#include <cmath>
#include "algo.h"

// Resultat från en serie mätningar för en viss algoritm + N.
struct sample {
    int    n;        // Storlek på testdatan
    double mean;     // Medelvärde av tider (sekunder)
    double stddev;   // Standardavvikelse (sekunder)
    int    samples;  // Antal mätpunkter (=iterations)
};

// =====================================================================
// EN MÄTNING
// =====================================================================
// Vi gör 'iterations' separata mätningar och beräknar mean/stddev över
// dem. Varje mätning får FRESH data - annars vore de andra mätningarna
// "sortering av redan sorterad data" vilket är ett helt annat scenario.
//
// För små N kan en enskild sortering vara så snabb att klockans
// upplösning (några hundra ns) blir en stor felkälla. Då används
// 'batch_size': vi mäter tiden för 'batch_size' sorteringar i rad och
// delar resultatet med batch_size. Det ger ett stabilt medelvärde per
// sortering utan att klockans precision dominerar.
inline sample run_benchmark(const algo& a, int n, int iterations = 10, int batch_size = 1) {
    std::vector<double> times;
    times.reserve(iterations);

    for (int i = 0; i < iterations; ++i) {
        // Förbered alla batchens arrayer i förväg så att datagenereringen
        // INTE räknas in i den uppmätta tiden.
        std::vector<vec_t> batch(batch_size);
        for (auto& b : batch) b = a.generator(n);

        auto start = std::chrono::steady_clock::now();
        for (auto& b : batch) {
            a.sort(b.begin(), b.end());
        }
        auto end = std::chrono::steady_clock::now();

        double total = std::chrono::duration<double>(end - start).count();
        times.push_back(total / batch_size);
    }

    // Medelvärde
    double sum = 0.0;
    for (double t : times) sum += t;
    double mean = sum / times.size();

    // Standardavvikelse med Bessels korrigering (N-1) eftersom vi har
    // ett urval, inte hela populationen. Detta är standard för
    // experimentella mätningar.
    double sq_sum = 0.0;
    for (double t : times) sq_sum += (t - mean) * (t - mean);
    double stddev = times.size() > 1 ? std::sqrt(sq_sum / (times.size() - 1)) : 0.0;

    return {n, mean, stddev, (int)times.size()};
}

// =====================================================================
// HEURISTIK FÖR BATCH-STORLEK
// =====================================================================
// Tumregel: vi vill att en mätperiod ska vara åtminstone några
// hundra mikrosekunder för att klockans brus inte ska dominera.
// För små N batchar vi flera sorteringar. För stora N räcker en.
inline int suggest_batch_size(int n) {
    if (n < 1000)   return 50;
    if (n < 5000)   return 10;
    if (n < 20000)  return 3;
    return 1;
}

#pragma once
#include <vector>
#include "algo.h"

// Resultat från en serie mätningar för en viss algoritm + N.
struct sample {
    int    n;        // Storlek på testdatan
    double mean;     // Medelvärde av tiderna (sekunder)
    double stddev;   // Standardavvikelse (sekunder)
    int    samples;  // Antal mätpunkter
};

// Kör 'iterations' mätningar (labbinstruktionen kräver minst 5 - vi
// defaultar till 10) och returnerar mean + stddev. Varje iteration får
// FRESH data så vi inte oavsiktligt mäter sortering av redan sorterad
// data.
//
// För små N kan en enskild sortering vara så snabb att klockans
// upplösning blir en stor felkälla. Då används 'batch_size': vi
// sorterar flera arrayer i samma tidsmätning och delar med batchens
// storlek. Datageneringen för batchen sker FÖRE tidsmätningen.
sample run_benchmark(const algo& experiment, int n,
                     int iterations = 10, int batch_size = 1);

// Heuristik som ger lämplig batch-storlek för ett givet N.
// Tumregel: vi vill att en mätperiod ska vara minst några hundra
// mikrosekunder för att klockans brus inte ska dominera resultatet.
int suggest_batch_size(int n);

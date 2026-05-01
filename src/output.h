#pragma once
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include "benchmark.h"

// =====================================================================
// CSV-EXPORT
// =====================================================================
// Vi sparar bara fyra kolumner per N-värde: själva storleken, medel,
// stddev och antalet samples. Det räcker för att rita kurvor med
// felstaplar - att spara ALLA enskilda mätningar är overkill för
// rapporten och gör filerna onödigt stora.
//
// Vetenskaplig notation används så att vi behåller precision för
// både små (mikrosekunder) och stora (sekunder) värden i samma fil.
inline void write_csv(const std::string& filename, const std::vector<sample>& data) {
    std::ofstream out(filename);
    out << "N,T,Stdev,Samples\n";
    out << std::scientific << std::setprecision(6);
    for (const auto& s : data) {
        out << s.n << "," << s.mean << "," << s.stddev << "," << s.samples << "\n";
    }
}

// =====================================================================
// KONSOLUTSKRIFT
// =====================================================================
// Snygg progressrapport medan benchmarken kör - viktigt eftersom hela
// körningen kan ta flera minuter. Annars sitter man och undrar om det
// hängt sig.
inline void print_row(const std::string& label, const sample& s) {
    std::cout << std::left  << std::setw(32) << label
              << std::right << "  N=" << std::setw(7) << s.n
              << "  mean="   << std::scientific << std::setprecision(3) << s.mean << " s"
              << "  stddev=" << s.stddev << " s"
              << "  (n="     << s.samples << ")\n";
}

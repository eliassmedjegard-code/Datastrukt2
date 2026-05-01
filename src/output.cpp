#include "output.h"
#include <fstream>
#include <iostream>
#include <iomanip>

void write_csv(const std::string& filename,
               const std::string& method_name,
               const std::vector<sample>& data) {
    std::ofstream out(filename);
    // # på första raden gör att gnuplot, pandas, numpy.loadtxt m.fl.
    // automatiskt hoppar över raden som kommentar.
    out << "# method=" << method_name << "\n";
    out << "N,T[ms],Stdev[ms],Samples\n";
    out << std::fixed << std::setprecision(6);
    for (const auto& measurement : data) {
        // Multiplicera med 1000 för att gå från sekunder till ms.
        out << measurement.n << ","
            << measurement.mean   * 1000.0 << ","
            << measurement.stddev * 1000.0 << ","
            << measurement.samples << "\n";
    }
}

void print_row(const std::string& label, const sample& s) {
    std::cout << std::left  << std::setw(32) << label
              << std::right << "  N=" << std::setw(7) << s.n
              << "  mean="   << std::scientific << std::setprecision(3) << s.mean << " s"
              << "  stddev=" << s.stddev << " s"
              << "  (n=" << s.samples << ")\n";
}

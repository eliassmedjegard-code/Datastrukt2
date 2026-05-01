#include "benchmark.h"
#include <chrono>
#include <cmath>

sample run_benchmark(const algo& experiment, int n,
                     int iterations, int batch_size) {
    std::vector<double> times;
    times.reserve(iterations);

    for (int iter = 0; iter < iterations; ++iter) {
        // Förbered alla batchens arrayer FÖRE tidsmätningen så att
        // datagenereringen inte räknas in i tiden. Det här löser den
        // klassiska fallgropen i avsnitt 6 i labbinstruktionen
        // ("man mäter inte endast sorteringssteget utan även
        // initierande programkod").
        std::vector<vec_t> batch(batch_size);
        for (auto& data : batch) data = experiment.generator(n);

        auto start = std::chrono::steady_clock::now();
        for (auto& data : batch) {
            // Vi skickar iteratorer (begin/end) till sorten - INGEN
            // kopia av datan går genom funktionsanropet, vilket annars
            // hade lagt till O(N) per sortering till mätningen.
            experiment.sort(data.begin(), data.end());
        }
        auto end = std::chrono::steady_clock::now();

        double total = std::chrono::duration<double>(end - start).count();
        times.push_back(total / batch_size);
    }

    // Medelvärde
    double sum = 0.0;
    for (double t : times) sum += t;
    double mean = sum / times.size();

    // Standardavvikelse med Bessels korrigering (1/(N-1)) eftersom vi
    // har ett urval, inte hela populationen. Detta är formel (1) i
    // labbinstruktionens avsnitt 9.2.
    double sq_sum = 0.0;
    for (double t : times) sq_sum += (t - mean) * (t - mean);
    double stddev = times.size() > 1
                  ? std::sqrt(sq_sum / (times.size() - 1))
                  : 0.0;

    return {n, mean, stddev, static_cast<int>(times.size())};
}

int suggest_batch_size(int n) {
    if (n < 1000)   return 50;
    if (n < 5000)   return 10;
    if (n < 20000)  return 3;
    return 1;
}

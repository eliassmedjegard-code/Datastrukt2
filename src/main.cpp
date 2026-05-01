#include <iostream>
#include <iomanip>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

#include "algo.h"
#include "sorting.h"
#include "generators.h"
#include "benchmark.h"
#include "output.h"

namespace fs = std::filesystem;

// =====================================================================
// STORLEKSUPPSÄTTNINGAR
// =====================================================================
// Labbinstruktionen kräver att N täcker minst en tiopotens.
//
// N_SLOW (faktor 50): för O(N^2)-algoritmer som insertion/selection.
// Vid N=50000 tar selection sort ~1-2 sekunder per sortering, vilket
// gör mätserien rimligt lång men inte outhärdlig.
//
// N_FAST (faktor 500): för O(N log N)-algoritmer. Här kan vi gå upp
// till 500000 utan problem på en sekund per sortering.
//
// N_RECURSIVE (faktor 40): för naiv quicksort på sorterad/konstant
// data. Här har vi BÅDE O(N^2) tid OCH O(N) rekursionsdjup, vilket
// betyder att vi riskerar stack overflow vid stora N. 20000 är säkert
// (~4 MB stack, default är 8 MB).
const std::vector<int> N_SLOW      = {1000, 2000, 5000, 10000, 20000, 50000};
const std::vector<int> N_FAST      = {1000, 5000, 10000, 50000, 100000, 500000};
const std::vector<int> N_RECURSIVE = {500, 1000, 2000, 5000, 10000, 20000};

// =====================================================================
// REGISTRERING AV ALGORITMER
// =====================================================================
// Här bygger vi upp listan över alla 20 experiment (5 algoritmer x 4
// datatyper). Varje algo-kombination väljer själv vilket N-set som är
// rimligt - t.ex. naiv quicksort på sorterad data är O(N^2) med djup
// rekursion och får därför N_RECURSIVE.
std::vector<algo> build_algorithms() {
    // Vi wrappar varje template-funktion i en lambda så att den matchar
    // sort_fn-signaturen. std::function kan inte hålla en oinstantierad
    // template direkt - den behöver en konkret typ att binda mot.
    sort_fn ins   = [](vec_t::iterator first, vec_t::iterator last) { insertion_sort(first, last); };
    sort_fn sel   = [](vec_t::iterator first, vec_t::iterator last) { selection_sort(first, last); };
    sort_fn qs    = [](vec_t::iterator first, vec_t::iterator last) { quicksort(first, last); };
    sort_fn qsm3  = [](vec_t::iterator first, vec_t::iterator last) { quicksort_m3(first, last); };
    sort_fn ssort = [](vec_t::iterator first, vec_t::iterator last) { std::sort(first, last); };

    return {
        // ---- Insertion sort: O(N^2) i värsta fall, ingen rekursion ----
        {"Insertion, slumpdata",   "insertion.random",      ins, random_gen,     N_SLOW},
        {"Insertion, stigande",    "insertion.rising",      ins, increasing_gen, N_SLOW},
        {"Insertion, fallande",    "insertion.falling",     ins, decreasing_gen, N_SLOW},
        {"Insertion, konstant",    "insertion.constant",    ins, constant_gen,   N_SLOW},

        // ---- Selection sort: ALLTID O(N^2), ingen rekursion ----
        {"Selection, slumpdata",   "selection.random",      sel, random_gen,     N_SLOW},
        {"Selection, stigande",    "selection.rising",      sel, increasing_gen, N_SLOW},
        {"Selection, fallande",    "selection.falling",     sel, decreasing_gen, N_SLOW},
        {"Selection, konstant",    "selection.constant",    sel, constant_gen,   N_SLOW},

        // ---- Naiv quicksort: O(N log N) på random, O(N^2) + O(N) djup på övriga ----
        {"Quicksort, slumpdata",   "quicksort.random",      qs, random_gen,     N_FAST},
        {"Quicksort, stigande",    "quicksort.rising",      qs, increasing_gen, N_RECURSIVE},
        {"Quicksort, fallande",    "quicksort.falling",     qs, decreasing_gen, N_RECURSIVE},
        {"Quicksort, konstant",    "quicksort.constant",    qs, constant_gen,   N_RECURSIVE},

        // ---- Quicksort m3: räddar sorterad data, men inte konstant ----
        {"Quicksort m3, slumpdata","quicksort_m3.random",   qsm3, random_gen,     N_FAST},
        {"Quicksort m3, stigande", "quicksort_m3.rising",   qsm3, increasing_gen, N_FAST},
        {"Quicksort m3, fallande", "quicksort_m3.falling",  qsm3, decreasing_gen, N_FAST},
        {"Quicksort m3, konstant", "quicksort_m3.constant", qsm3, constant_gen,   N_RECURSIVE},

        // ---- std::sort: introsort, hanterar alla fall i O(N log N) ----
        {"std::sort, slumpdata",   "std_sort.random",       ssort, random_gen,     N_FAST},
        {"std::sort, stigande",    "std_sort.rising",       ssort, increasing_gen, N_FAST},
        {"std::sort, fallande",    "std_sort.falling",      ssort, decreasing_gen, N_FAST},
        {"std::sort, konstant",    "std_sort.constant",     ssort, constant_gen,   N_FAST},
    };
}

// =====================================================================
// SANITY CHECK
// =====================================================================
// Innan vi spenderar minuter på benchmark är det värt att verifiera
// att alla sorter faktiskt sorterar. Annars kan en bugg ge "snygga"
// tider utan att vi märker att resultatet är trasigt.
bool verify_all(const std::vector<algo>& experiments) {
    bool ok = true;
    for (const auto& experiment : experiments) {
        vec_t data = experiment.generator(100);
        experiment.sort(data.begin(), data.end());
        if (!std::is_sorted(data.begin(), data.end())) {
            std::cerr << "FEL: '" << experiment.id
                      << "' producerar inte sorterat resultat!\n";
            ok = false;
        }
    }
    return ok;
}

// Kör en algoritm för alla dess N-värden och spara CSV.
void run_and_save(const algo& experiment, const std::string& outdir) {
    std::cout << "\n=== " << experiment.title
              << " (" << experiment.id << ") ===\n";
    std::vector<sample> results;
    for (int n : experiment.sizes) {
        int batch = suggest_batch_size(n);
        sample s = run_benchmark(experiment, n, /*iterations=*/10, batch);
        print_row(experiment.title, s);
        results.push_back(s);
    }
    std::string path = outdir + "/" + experiment.id + ".csv";
    write_csv(path, experiment.title, results);
    std::cout << "  -> sparat till " << path << "\n";
}

void print_usage(const std::vector<algo>& experiments) {
    std::cout << "Användning:\n"
              << "  sortlab all             - kör alla 20 experiment\n"
              << "  sortlab <id> [<id>...]  - kör bara angivna experiment\n"
              << "  sortlab list            - lista tillgängliga id:n\n\n"
              << "Exempel:\n"
              << "  sortlab quicksort.random std_sort.random\n\n"
              << "Tillgängliga experiment:\n";
    for (const auto& experiment : experiments) {
        std::cout << "  " << std::left << std::setw(28) << experiment.id
                  << "  " << experiment.title << "\n";
    }
}

int main(int argc, char* argv[]) {
    auto experiments = build_algorithms();

    // Skapar results/ om den inte finns. create_directories är safe
    // även om mappen redan finns - returnerar då bara false.
    const std::string outdir = "results";
    fs::create_directories(outdir);

    if (argc == 1) {
        print_usage(experiments);
        return 0;
    }

    std::string first_arg = argv[1];
    if (first_arg == "list") {
        print_usage(experiments);
        return 0;
    }

    std::cout << "Verifierar sorteringar...\n";
    if (!verify_all(experiments)) {
        std::cerr << "Avbryter pga felaktig sortering.\n";
        return 1;
    }
    std::cout << "OK.\n";

    if (first_arg == "all") {
        for (const auto& experiment : experiments) run_and_save(experiment, outdir);
        return 0;
    }

    // Kör bara de experiment användaren bett om
    for (int arg_index = 1; arg_index < argc; ++arg_index) {
        auto match = std::find_if(experiments.begin(), experiments.end(),
            [&](const algo& experiment) { return experiment.id == argv[arg_index]; });
        if (match == experiments.end()) {
            std::cerr << "Okänt experiment-id: " << argv[arg_index] << "\n";
            continue;
        }
        run_and_save(*match, outdir);
    }
    return 0;
}

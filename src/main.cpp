#include <iostream>
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
// Vi har två N-set. Anledningen är att O(N^2)-algoritmer blir
// outhärdligt långsamma vid N=500000 (skulle ta timmar), så för dem
// stannar vi vid N=10000.
//
// Det är medvetet att de två seten har en överlappning vid N=1000-10000
// så att vi kan jämföra alla algoritmer mot varandra för "mittenstora"
// arrayer.
const std::vector<int> N_SLOW = {500, 1000, 2000, 5000, 10000};
const std::vector<int> N_FAST = {1000, 5000, 10000, 50000, 100000, 500000};

// =====================================================================
// REGISTRERING AV ALGORITMER
// =====================================================================
// Här bygger vi upp listan över alla 20 experiment (5 algoritmer x 4
// datatyper). Varje algo-kombination väljer själv vilket N-set som är
// rimligt - t.ex. naiv quicksort på sorterad data är O(N^2) och får
// därför N_SLOW.
std::vector<algo> build_algorithms() {
    // Wrappar varje template-funktion i en lambda så att den matchar
    // sort_fn-signaturen (std::function kan inte hålla en oinstantierad
    // template direkt).
    sort_fn ins   = [](vec_t::iterator a, vec_t::iterator b){ insertion_sort(a, b); };
    sort_fn sel   = [](vec_t::iterator a, vec_t::iterator b){ selection_sort(a, b); };
    sort_fn qs    = [](vec_t::iterator a, vec_t::iterator b){ quicksort(a, b); };
    sort_fn qsm3  = [](vec_t::iterator a, vec_t::iterator b){ quicksort_m3(a, b); };
    sort_fn ssort = [](vec_t::iterator a, vec_t::iterator b){ std::sort(a, b); };

    return {
        // ---- Insertion sort: O(N^2) i värsta fall, så N_SLOW för alla ----
        {"Insertion, slumpdata",   "insertion.random",      ins, random_gen,     N_SLOW},
        {"Insertion, stigande",    "insertion.rising",      ins, increasing_gen, N_SLOW},
        {"Insertion, fallande",    "insertion.falling",     ins, decreasing_gen, N_SLOW},
        {"Insertion, konstant",    "insertion.constant",    ins, constant_gen,   N_SLOW},

        // ---- Selection sort: ALLTID O(N^2), N_SLOW för alla ----
        {"Selection, slumpdata",   "selection.random",      sel, random_gen,     N_SLOW},
        {"Selection, stigande",    "selection.rising",      sel, increasing_gen, N_SLOW},
        {"Selection, fallande",    "selection.falling",     sel, decreasing_gen, N_SLOW},
        {"Selection, konstant",    "selection.constant",    sel, constant_gen,   N_SLOW},

        // ---- Naiv quicksort: O(N^2) på sorterad/konstant, O(NlogN) på random ----
        {"Quicksort, slumpdata",   "quicksort.random",      qs, random_gen,     N_FAST},
        {"Quicksort, stigande",    "quicksort.rising",      qs, increasing_gen, N_SLOW},
        {"Quicksort, fallande",    "quicksort.falling",     qs, decreasing_gen, N_SLOW},
        {"Quicksort, konstant",    "quicksort.constant",    qs, constant_gen,   N_SLOW},

        // ---- Quicksort m3: räddar sorterad data, men inte konstant ----
        {"Quicksort m3, slumpdata","quicksort_m3.random",   qsm3, random_gen,     N_FAST},
        {"Quicksort m3, stigande", "quicksort_m3.rising",   qsm3, increasing_gen, N_FAST},
        {"Quicksort m3, fallande", "quicksort_m3.falling",  qsm3, decreasing_gen, N_FAST},
        {"Quicksort m3, konstant", "quicksort_m3.constant", qsm3, constant_gen,   N_SLOW},

        // ---- std::sort: introsort, hanterar alla fall i O(NlogN) ----
        {"std::sort, slumpdata",   "std_sort.random",       ssort, random_gen,     N_FAST},
        {"std::sort, stigande",    "std_sort.rising",       ssort, increasing_gen, N_FAST},
        {"std::sort, fallande",    "std_sort.falling",      ssort, decreasing_gen, N_FAST},
        {"std::sort, konstant",    "std_sort.constant",     ssort, constant_gen,   N_FAST},
    };
}

// =====================================================================
// SANITY CHECK
// =====================================================================
// Innan vi kör flera minuters benchmark är det värt att verifiera att
// alla sorter faktiskt sorterar. Annars kan en bugg ge "snygga" tider
// utan att vi märker att resultatet är fel.
bool verify_all(const std::vector<algo>& algos) {
    bool ok = true;
    for (const auto& a : algos) {
        // Använder generator från algon själv så vi täcker alla datatyper.
        vec_t v = a.generator(100);
        a.sort(v.begin(), v.end());
        if (!std::is_sorted(v.begin(), v.end())) {
            std::cerr << "FEL: '" << a.id << "' producerar inte sorterat resultat!\n";
            ok = false;
        }
    }
    return ok;
}

// Kör en algoritm för alla dess N-värden och spara CSV.
void run_and_save(const algo& a, const std::string& outdir) {
    std::cout << "\n=== " << a.title << " (" << a.id << ") ===\n";
    std::vector<sample> results;
    for (int n : a.sizes) {
        int batch = suggest_batch_size(n);
        sample s = run_benchmark(a, n, /*iterations=*/10, /*batch_size=*/batch);
        print_row(a.title, s);
        results.push_back(s);
    }
    std::string path = outdir + "/" + a.id + ".csv";
    write_csv(path, results);
    std::cout << "  -> sparat till " << path << "\n";
}

void print_usage(const std::vector<algo>& all) {
    std::cout << "Användning:\n"
              << "  sortlab all             - kör alla 20 experiment\n"
              << "  sortlab <id> [<id>...]  - kör bara angivna experiment\n"
              << "  sortlab list            - lista tillgängliga id:n\n\n"
              << "Exempel:\n"
              << "  sortlab quicksort.random std_sort.random\n\n"
              << "Tillgängliga experiment:\n";
    for (const auto& a : all) {
        std::cout << "  " << std::left << std::setw(28) << a.id << "  " << a.title << "\n";
    }
}

int main(int argc, char* argv[]) {
    auto algorithms = build_algorithms();

    // Skapar results/ om den inte finns. filesystem::create_directories är
    // safe även om mappen redan finns - returnerar bara false då.
    const std::string outdir = "results";
    fs::create_directories(outdir);

    if (argc == 1) {
        print_usage(algorithms);
        return 0;
    }

    std::string first = argv[1];
    if (first == "list") {
        print_usage(algorithms);
        return 0;
    }

    // Verifiera att sorterna fungerar innan vi spenderar minuter på mätning.
    std::cout << "Verifierar sorteringar...\n";
    if (!verify_all(algorithms)) {
        std::cerr << "Avbryter pga felaktig sortering.\n";
        return 1;
    }
    std::cout << "OK.\n";

    if (first == "all") {
        for (const auto& a : algorithms) run_and_save(a, outdir);
        return 0;
    }

    // Kör bara de experiment användaren bett om
    for (int i = 1; i < argc; ++i) {
        auto it = std::find_if(algorithms.begin(), algorithms.end(),
            [&](const algo& a){ return a.id == argv[i]; });
        if (it == algorithms.end()) {
            std::cerr << "Okänt experiment-id: " << argv[i] << "\n";
            continue;
        }
        run_and_save(*it, outdir);
    }
    return 0;
}

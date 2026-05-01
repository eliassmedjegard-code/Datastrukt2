#pragma once
#include <functional>
#include <vector>
#include <string>

// Vi alias:ar typerna här eftersom de dyker upp överallt i koden.
// Att skriva ut std::function<void(std::vector<int>::iterator, ...)>
// varje gång hade gjort koden oläsbar.
using vec_t = std::vector<int>;
using sort_fn = std::function<void(vec_t::iterator, vec_t::iterator)>;
using gen_fn  = std::function<vec_t(int)>;

// algo-strukturen är hjärtat i designen. Tanken är att INTE skriva
// 20 separata main-funktioner för 5 algoritmer x 4 datatyper, utan
// istället se varje kombination som ett "experiment" i en lista som
// vi kan iterera över.
//
// std::function används istället för funktionspekare/templates för att
// vi ska kunna lagra olika typer av sort/generator i samma container.
// Det kostar lite overhead per anrop men det är konstant overhead för
// alla algoritmer, så jämförelser mellan dem är fortfarande rättvisa.
struct algo {
    std::string title;          // Människoläsbart namn för konsolutskrift
    std::string id;             // CLI-identifierare och CSV-filnamn (t.ex. "quicksort.random")
    sort_fn sort;               // Själva sorteringen
    gen_fn  generator;          // Genererar testdata av given storlek
    std::vector<int> sizes;     // Vilka N-värden vi ska mäta
};

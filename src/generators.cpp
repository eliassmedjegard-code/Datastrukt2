#include "generators.h"
#include <random>
#include <numeric>

vec_t random_gen(int n) {
    // Statisk RNG - vi seedar EN gång och får sedan en stadig ström av
    // slumpvärden tack vare RNG:ns interna tillstånd. Att seeda om för
    // varje anrop hade gett upprepade sekvenser för anrop nära varandra
    // i tid (random_device har låg upplösning).
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(0, n);
    vec_t data(n);
    for (auto& value : data) value = dist(rng);
    return data;
}

vec_t increasing_gen(int n) {
    vec_t data(n);
    std::iota(data.begin(), data.end(), 0);
    return data;
}

vec_t decreasing_gen(int n) {
    vec_t data(n);
    // iota på rbegin/rend ger fallande sekvens utan extra reverse-steg.
    std::iota(data.rbegin(), data.rend(), 0);
    return data;
}

vec_t constant_gen(int n) {
    return vec_t(n, 42);
}

#pragma once
#include <algorithm>
#include <iterator>

// Alla sorter är templates på iteratortyp. Det gör dem generella, men
// huvudpoängen är att vi kan testa på vec_t::iterator i denna labb
// utan att behöva tänka på typkonverteringar.

// =====================================================================
// INSERTION SORT
// =====================================================================
// Tidskomplexitet: O(N^2) värsta fall, O(N) bästa fall (redan sorterad).
// Vi använder "shift" istället för "swap" inne i loopen - det halverar
// ungefär antalet minnesoperationer eftersom vi bara skriver varje
// element en gång istället för tre gånger per swap.
template<typename It>
void insertion_sort(It first, It last) {
    for (It i = first + 1; i < last; ++i) {
        auto val = *i;
        It j = i;
        while (j > first && *(j - 1) > val) {
            *j = *(j - 1);
            --j;
        }
        *j = val;
    }
}

// =====================================================================
// SELECTION SORT
// =====================================================================
// Tidskomplexitet: ALLTID O(N^2), oavsett indata. Det är ett "rättvist"
// jämförelseunderlag eftersom datatypen inte spelar roll - vi kan
// observera om mätningarna är någotsånär konsistenta över de fyra
// generatorerna (de borde vara det).
template<typename It>
void selection_sort(It first, It last) {
    for (It i = first; i < last; ++i) {
        It min_it = i;
        for (It j = i + 1; j < last; ++j) {
            if (*j < *min_it) min_it = j;
        }
        std::iter_swap(i, min_it);
    }
}

// =====================================================================
// QUICKSORT - hjälpare: Lomuto-partition
// =====================================================================
// Lomuto är enklare att skriva korrekt än Hoare. Pivoten är sista
// elementet. Returnerar iterator till pivotens slutposition.
//
// Notera: Lomuto har en känd svaghet med många lika element - den ger
// O(N^2) när alla element är lika, eftersom alla går åt samma håll.
// Detta är ett av fenomenen vi VILL observera i labben.
template<typename It>
It partition_lomuto(It first, It last) {
    auto pivot = *(last - 1);
    It i = first;
    for (It j = first; j < last - 1; ++j) {
        if (*j <= pivot) {
            std::iter_swap(i, j);
            ++i;
        }
    }
    std::iter_swap(i, last - 1);
    return i;
}

// =====================================================================
// QUICKSORT med naiv pivot (sista elementet)
// =====================================================================
// Tidskomplexitet: O(N log N) i medel, O(N^2) i värsta fall.
// Värsta fall inträffar för redan sorterad data (stigande/fallande)
// eftersom pivoten då alltid är max- eller minelementet, vilket ger
// helt obalanserade partitioner. Ett av huvudfynden i labben.
template<typename It>
void quicksort(It first, It last) {
    if (last - first <= 1) return;
    It p = partition_lomuto(first, last);
    quicksort(first, p);
    quicksort(p + 1, last);
}

// =====================================================================
// QUICKSORT med median-of-three pivot
// =====================================================================
// Vi tar medianen av första, mittersta och sista elementet och
// använder den som pivot. För redan sorterad data blir då pivoten
// det riktiga mittenelementet -> perfekt balans -> O(N log N).
//
// Detta löser worst case för stigande/fallande data men hjälper INTE
// mot konstant data, eftersom alla element är lika och Lomuto-loopen
// fortfarande hamnar i obalans.
template<typename It>
void median_of_three(It first, It last) {
    It mid    = first + (last - first) / 2;
    It lastEl = last - 1;

    // Sortera trippeln (first, mid, lastEl) med tre jämförelser så vi
    // vet vem som är medianen.
    if (*first > *mid)    std::iter_swap(first, mid);
    if (*first > *lastEl) std::iter_swap(first, lastEl);
    if (*mid   > *lastEl) std::iter_swap(mid, lastEl);
    // Invariant nu: *first <= *mid <= *lastEl, dvs medianen är på 'mid'.

    // Flytta medianen till sista positionen så att partition_lomuto
    // (som plockar pivot från last-1) får rätt element som pivot.
    std::iter_swap(mid, lastEl);
}

template<typename It>
void quicksort_m3(It first, It last) {
    if (last - first <= 1) return;

    // median_of_three behöver minst 3 element. För 2 element räcker det
    // med en jämförelse och eventuell swap.
    if (last - first == 2) {
        if (*first > *(first + 1)) std::iter_swap(first, first + 1);
        return;
    }

    median_of_three(first, last);
    It p = partition_lomuto(first, last);
    quicksort_m3(first, p);
    quicksort_m3(p + 1, last);
}

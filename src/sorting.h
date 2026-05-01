#pragma once
#include <algorithm>
#include <iterator>

// Alla sorter är templates på iteratortyp - de skrivs därför direkt i
// headern (templates kan inte ligga i cpp-filer eftersom kompilatorn
// behöver se hela definitionen vid varje instansiering).
//
// Variabelnamn är medvetet beskrivande snarare än enbokstaviga
// (current, hole, scan, ...) för att läsbarheten ska bli bra.

// =====================================================================
// INSERTION SORT
// =====================================================================
// Tidskomplexitet: O(N^2) värsta fall, O(N) bästa fall (redan sorterad).
// Vi använder "shift" istället för "swap" - varje element skrivs bara
// en gång istället för tre per swap, vilket halverar antalet
// minnesoperationer.
template<typename It>
void insertion_sort(It first, It last) {
    for (It current = first + 1; current < last; ++current) {
        auto value = *current;
        It hole = current;
        while (hole > first && *(hole - 1) > value) {
            *hole = *(hole - 1);
            --hole;
        }
        *hole = value;
    }
}

// =====================================================================
// SELECTION SORT
// =====================================================================
// Tidskomplexitet: ALLTID O(N^2), oavsett indata.
// Bra som referens - mätningarna borde vara mycket lika över alla fyra
// generatorerna eftersom datatypen inte påverkar arbetsmängden.
template<typename It>
void selection_sort(It first, It last) {
    for (It boundary = first; boundary < last; ++boundary) {
        It min_pos = boundary;
        for (It scan = boundary + 1; scan < last; ++scan) {
            if (*scan < *min_pos) min_pos = scan;
        }
        std::iter_swap(boundary, min_pos);
    }
}

// =====================================================================
// LOMUTO-PARTITION (hjälpare till båda quicksort-varianterna)
// =====================================================================
// Pivoten är ALLTID sista elementet. Detta är medvetet - båda
// quicksort-varianterna delar samma partition, och median-of-three
// arrangerar bara om datan så att medianen hamnar sist före anropet.
//
// Lomutos kända svaghet: när alla element är lika hamnar de alla i
// vänstra partitionen → O(N^2). Detta är fenomenet vi observerar med
// constant_gen.
template<typename It>
It partition_lomuto(It first, It last) {
    auto pivot = *(last - 1);
    It store_pos = first;
    for (It scan = first; scan < last - 1; ++scan) {
        if (*scan <= pivot) {
            std::iter_swap(store_pos, scan);
            ++store_pos;
        }
    }
    std::iter_swap(store_pos, last - 1);
    return store_pos;
}

// =====================================================================
// QUICKSORT med naiv pivot (höger element)
// =====================================================================
// Tidskomplexitet: O(N log N) i medel, O(N^2) i värsta fall.
// Värsta fall = redan sorterad data: pivoten är då alltid max- eller
// minelementet → helt obalanserade partitioner → O(N^2) och O(N) djup
// rekursion (risk för stack overflow vid stora N).
template<typename It>
void quicksort(It first, It last) {
    if (last - first <= 1) return;
    It split = partition_lomuto(first, last);
    quicksort(first, split);
    quicksort(split + 1, last);
}

// =====================================================================
// MEDIAN-OF-THREE (hjälpare till quicksort_m3)
// =====================================================================
// Vi tar medianen av första, mittersta och sista elementet och
// placerar den på sista positionen så att partition_lomuto plockar
// upp den som pivot.
//
// För redan sorterad data blir då pivoten det riktiga mittenelementet
// → perfekt partitionering → O(N log N) istället för O(N^2). Det är
// hela poängen med median-of-three.
//
// OBS: hjälper INTE mot constant_gen eftersom alla element är lika -
// Lomuto-loopen hamnar i obalans oavsett vilken pivot vi väljer.
template<typename It>
void median_of_three(It first, It last) {
    It mid       = first + (last - first) / 2;
    It last_pos  = last - 1;

    if (*first > *mid)      std::iter_swap(first, mid);
    if (*first > *last_pos) std::iter_swap(first, last_pos);
    if (*mid   > *last_pos) std::iter_swap(mid, last_pos);
    // Invariant nu: *first <= *mid <= *last_pos, dvs medianen är på 'mid'.

    // Flytta medianen sist så partition_lomuto kan plocka upp den.
    std::iter_swap(mid, last_pos);
}

template<typename It>
void quicksort_m3(It first, It last) {
    if (last - first <= 1) return;

    // Specialfall: median_of_three behöver minst 3 element.
    if (last - first == 2) {
        if (*first > *(first + 1)) std::iter_swap(first, first + 1);
        return;
    }

    median_of_three(first, last);
    It split = partition_lomuto(first, last);
    quicksort_m3(first, split);
    quicksort_m3(split + 1, last);
}

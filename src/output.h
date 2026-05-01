#pragma once
#include <string>
#include <vector>
#include "benchmark.h"

// Sparar mätserien som CSV-liknande textfil. Tider rapporteras i
// millisekunder för att matcha labbinstruktionens exempelformat.
//
// Första raden är en kommentar (# method=...) som anger vilken
// sorteringsmetod datat kommer ifrån - lab-kravet på datarapportering
// säger att "sorteringsmetod" ska finnas med.
void write_csv(const std::string& filename,
               const std::string& method_name,
               const std::vector<sample>& data);

// Snygg konsolprogress under körning - viktig eftersom hela
// benchmark-svängen kan ta många minuter.
void print_row(const std::string& label, const sample& s);

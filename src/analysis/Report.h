#pragma once
// Generación del informe (Markdown, se abre directamente en Obsidian) y del CSV del barrido.
#include "analysis/Analysis.h"
#include <string>
#include <vector>

namespace susp {

struct ReportInput {
    const Suspension* susp = nullptr;
    Metrics staticMetrics;           // posición de diseño (travel = 0, rack = 0)
    std::vector<Metrics> sweep;      // barrido de recorrido
    std::string notes;               // texto libre (p. ej. resultado de una optimización)
};

bool writeMarkdownReport(const std::string& path, const ReportInput& in, std::string& error);
bool writeSweepCsv(const std::string& path, const std::vector<Metrics>& sweep, std::string& error);

// Nombre de archivo sugerido: "Informe <nombre> <fecha hora>"
std::string defaultReportBaseName(const Suspension& s);

} // namespace susp

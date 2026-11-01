#pragma once
// =============================================================================
// Optimizer.h — Optimización de puntos duros respecto a un objetivo
// -----------------------------------------------------------------------------
// Método: Nelder-Mead (simplex) sin derivadas, con límites por recorte.
// Es sencillo y robusto para pocas variables (≲ 10). Ver docs/Simulación/Optimización.md
// para ampliarlo (multiobjetivo, restricciones, otros algoritmos...).
// =============================================================================
#include "analysis/Analysis.h"
#include <string>
#include <vector>

namespace susp {

// Una variable de diseño = una coordenada de un punto duro que puede moverse
struct DesignVariable {
    std::string hardpoint;  // nombre del JSON, p. ej. "tie_rod_inner"
    int axis = 2;           // 0 = X, 1 = Y, 2 = Z
    double minOffset = -20; // límite inferior respecto al valor inicial [mm]
    double maxOffset = +20; // límite superior respecto al valor inicial [mm]
};

enum class ObjectiveType {
    MinBumpSteer,          // minimizar la variación de convergencia en el recorrido
    TargetCamberGain,      // llevar la ganancia de caída [°/10 mm] a un objetivo
    TargetRollCenter,      // llevar la altura del CB estático a un objetivo [mm]
    MinRollCenterMigration // minimizar la variación de altura del CB en el recorrido
};
const char* objectiveName(ObjectiveType t);

struct Objective {
    ObjectiveType type = ObjectiveType::MinBumpSteer;
    double target = 0.0;
};

struct OptimizerSettings {
    int maxIterations = 300;
    double tolerance = 1e-8;
    int sweepSteps = 11;     // puntos del barrido usado en cada evaluación
};

struct OptimizerResult {
    bool ok = false;
    Suspension best;
    double initialCost = 0, finalCost = 0;
    int iterations = 0, evaluations = 0;
    std::vector<double> offsets;   // desplazamiento final de cada variable [mm]
    std::vector<double> history;   // mejor coste en cada iteración
    std::string message;
};

// Coste de una suspensión para un objetivo (menor = mejor)
double evaluateObjective(const Suspension& s, const Objective& obj, int sweepSteps);

OptimizerResult optimize(const Suspension& start, const std::vector<DesignVariable>& vars,
                         const Objective& obj, const OptimizerSettings& settings = {});

} // namespace susp

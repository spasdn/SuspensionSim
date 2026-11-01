#pragma once
// =============================================================================
// Analysis.h — Parámetros de la suspensión en una posición dada
// -----------------------------------------------------------------------------
// Definiciones y criterios de signo en docs/Teoría/Parámetros de suspensión.md
// Ángulos en grados, longitudes en mm.
// =============================================================================
#include "core/KinematicSolver.h"
#include <vector>

namespace susp {

struct Metrics {
    bool   valid = false;
    double travel = 0, rack = 0;

    // Alineación
    double camber = 0;        // caída        [°]  (- = parte superior hacia dentro)
    double toe = 0;           // convergencia [°]  (+ = toe-in)
    // Eje de pivote (kingpin)
    double caster = 0;        // avance       [°]  (+ = parte superior hacia atrás)
    double kpi = 0;           // inclinación del pivote [°] (+ = parte superior hacia dentro)
    double scrubRadius = 0;   // radio de pivotamiento en el suelo [mm] (+ = eje corta el suelo por dentro)
    double mechanicalTrail = 0;   // avance mecánico (trail) [mm] (+ = eje corta el suelo por delante)
    double kingpinOffsetWC = 0;   // brazo del pivote a la altura del centro de rueda [mm]
    // Centros instantáneos
    bool   fvicAtInfinity = false;
    Vec3   fvic{};            // centro instantáneo en vista frontal (x = x del centro de rueda)
    double fvsaLength = 0;    // longitud del brazo oscilante virtual frontal [mm]
    Vec3   rollCenter{};      // centro de balanceo (en el plano y = 0)
    double rollCenterHeight = 0;  // altura del CB sobre el suelo [mm]
    bool   svicAtInfinity = false;
    Vec3   svic{};            // centro instantáneo en vista lateral
    double svsaLength = 0;    // longitud del brazo oscilante virtual lateral [mm]
    double antiBrake = 0;     // anti-dive (del.) / anti-lift (tras.) en frenada [%]
    double antiAccel = 0;     // anti-lift (del.) / anti-squat (tras.) en aceleración [%]
    // Posición de rueda
    double halfTrack = 0;     // semivía en el contacto [mm]
    double wheelCenterX = 0;  // posición longitudinal del centro de rueda [mm]
    Vec3   contactPatch{};
    // Muelle-amortiguador
    double damperLength = 0;  // [mm]
    double motionRatio = 0;   // d(longitud amort.)/d(recorrido rueda) [-]
};

// Calcula todos los parámetros para una entrada (travel, rack).
// Usa el solver para obtener posiciones vecinas (ratio de instalación).
Metrics analyze(KinematicSolver& solver, const KinematicInputs& in);

// Parámetros a partir de una posición ya resuelta (sin ratio de instalación).
Metrics computeGeometry(const Suspension& s, const CornerState& st);

// Barrido de recorrido entre travelMin y travelMax (incluye el 0).
std::vector<Metrics> sweepTravel(KinematicSolver& solver, double travelMin, double travelMax,
                                 int steps, double rack);

// Derivada numérica de una magnitud del barrido en travel = 0 (p. ej. ganancia de caída)
double gradientAtZero(const std::vector<Metrics>& sweep, double Metrics::*field);

} // namespace susp

#pragma once
// =============================================================================
// KinematicSolver.h — Cinemática de una esquina de doble trapecio
// -----------------------------------------------------------------------------
// Idea (detalle en docs/Teoría/Solver cinemático.md):
//   1. El brazo inferior gira un ángulo θ alrededor de su eje  -> rótula inferior.
//   2. El brazo superior gira φ hasta que la distancia entre rótulas se conserva.
//   3. La rótula de la bieleta se obtiene cortando 3 esferas (mangueta rígida +
//      longitud de bieleta).
//   4. Con 3 puntos de la mangueta antes/después se saca la transformación rígida
//      y se mueve el resto (centro de rueda, eje de mangueta...).
//   5. Para imponer un recorrido de rueda dado se busca θ con Newton.
// =============================================================================
#include "core/Suspension.h"

namespace susp {

// Lo que el usuario impone (condiciones de contorno cinemáticas)
struct KinematicInputs {
    double travel = 0.0;  // recorrido vertical del centro de rueda [mm], + = compresión
    double rack   = 0.0;  // desplazamiento de la cremallera en Y [mm]
};

// Posición resuelta de todos los puntos móviles
struct CornerState {
    bool valid = false;
    double lowerAngle = 0.0;  // θ [rad]
    double upperAngle = 0.0;  // φ [rad]
    Vec3 lowerBallJoint{}, upperBallJoint{}, tieRodOuter{};
    Vec3 tieRodInner{};       // desplazado por la cremallera
    Vec3 wheelCenter{}, spindlePoint{}, contactPatch{};
    Vec3 damperArm{};
};

class KinematicSolver {
public:
    explicit KinematicSolver(const Suspension& s);

    // Resuelve para un recorrido y una cremallera dados. Usa la última solución
    // como punto de partida (arranque en caliente), así un barrido es rápido.
    CornerState solve(const KinematicInputs& in);

    // Resuelve con el ángulo del brazo inferior impuesto (paso interno).
    CornerState solveAtLowerAngle(double theta, double rack, const CornerState* guess) const;

    const CornerState& reference() const { return ref_; }  // posición estática
    const Suspension&  suspension() const { return s_; }
    void resetWarmStart() { last_ = ref_; }

    // Utilidades geométricas públicas (las usa también el análisis)
    static Vec3 rotateAboutAxis(const Vec3& p, const Vec3& a, const Vec3& b, double angle);
    static Vec3 contactPatchFrom(const Vec3& wc, const Vec3& sp, double radius);

private:
    Suspension s_;
    CornerState ref_;
    CornerState last_;
    double lKingpin_ = 0, lLbjTro_ = 0, lUbjTro_ = 0, lTieRod_ = 0;
};

} // namespace susp

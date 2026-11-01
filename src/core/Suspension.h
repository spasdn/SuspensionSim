#pragma once
// =============================================================================
// Suspension.h — Datos de entrada de una esquina de suspensión de doble trapecio
// -----------------------------------------------------------------------------
// Sistema de coordenadas ISO 8855 (ver docs/Teoría/Sistema de coordenadas.md):
//   X hacia delante, Y hacia la izquierda, Z hacia arriba. Unidades: mm.
// Se modela UNA esquina (p. ej. delantera izquierda). El lado opuesto se
// considera simétrico respecto al plano Y = 0.
// =============================================================================
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace susp {

using Vec3 = glm::dvec3;   // doble precisión para el cálculo

enum class Side { Left, Right };
enum class Axle { Front, Rear };
enum class Attach { LowerArm, UpperArm, Upright };

// Puntos duros (hardpoints). Los nombres entre comillas son las claves del JSON.
struct Hardpoints {
    // --- Fijos al chasis ---
    Vec3 lcaFront{};      // "lca_front"   brazo inferior, anclaje delantero
    Vec3 lcaRear{};       // "lca_rear"    brazo inferior, anclaje trasero
    Vec3 ucaFront{};      // "uca_front"   brazo superior, anclaje delantero
    Vec3 ucaRear{};       // "uca_rear"    brazo superior, anclaje trasero
    Vec3 tieRodInner{};   // "tie_rod_inner" bieleta de dirección, lado cremallera

    // --- Solidarios a la mangueta (upright) ---
    Vec3 lowerBallJoint{}; // "lower_ball_joint" rótula inferior
    Vec3 upperBallJoint{}; // "upper_ball_joint" rótula superior
    Vec3 tieRodOuter{};    // "tie_rod_outer"    rótula de la bieleta en la mangueta
    Vec3 wheelCenter{};    // "wheel_center"     centro de rueda
    Vec3 spindlePoint{};   // "spindle_point"    punto sobre el eje de la mangueta, HACIA FUERA

    // --- Muelle/amortiguador (opcional) ---
    bool hasDamper = false;
    Vec3 damperChassis{};  // "damper.chassis"
    Vec3 damperArm{};      // "damper.arm"
    Attach damperAttach = Attach::LowerArm;  // "damper.attached_to"
};

struct Tire {
    double radius = 300.0;  // radio bajo carga [mm]
    double width  = 205.0;  // ancho [mm] (solo para dibujar)
};

struct VehicleParams {
    double wheelbase      = 2600.0; // batalla [mm]
    double cgHeight       = 500.0;  // altura del CdG [mm]
    double mass           = 1200.0; // masa total [kg] (para futuras cargas)
    double frontBrakeBias = 0.65;   // fracción de frenada en el eje delantero
    double frontDriveFraction = 0.0;// fracción de tracción delantera (0 = propulsión, 1 = tracción)
};

// Condiciones de contorno por defecto del caso (se pueden cambiar en la interfaz)
struct BoundaryConditions {
    double travelMin = -60.0;  // extensión (rebote)  [mm]
    double travelMax =  60.0;  // compresión (bump)   [mm]
    int    steps     =  25;    // puntos del barrido
    double rack      =   0.0;  // desplazamiento de cremallera [mm] (+ hacia +Y)
    double rackMin   = -30.0;
    double rackMax   =  30.0;
};

struct Suspension {
    std::string name = "sin nombre";
    Axle axle = Axle::Front;
    Side side = Side::Left;
    Hardpoints hp;
    Tire tire;
    VehicleParams vehicle;
    BoundaryConditions bc;
    std::string chassisMeshPath;  // ruta absoluta a un .obj opcional

    // +1 para el lado izquierdo, -1 para el derecho: sirve para que los signos
    // de caída, convergencia, KPI... salgan con el criterio habitual en ambos lados.
    double sideSign() const { return side == Side::Left ? 1.0 : -1.0; }
};

// Acceso a un punto duro por su nombre del JSON (útil para el optimizador y la UI).
// Devuelve nullptr si el nombre no existe.
Vec3* hardpointByName(Hardpoints& hp, const std::string& name);
const std::vector<std::string>& hardpointNames();

} // namespace susp

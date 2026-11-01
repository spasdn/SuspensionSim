// Test de consola del núcleo de cálculo (no abre ventana).
// Ejecuta:  ctest   ó   ./test_kinematics [ruta.json]
#include "analysis/Analysis.h"
#include "analysis/Optimizer.h"
#include "io/SuspensionLoader.h"

#include <cmath>
#include <cstdio>
#include <string>

using namespace susp;

static int failures = 0;
#define CHECK(cond, msg)                                                   \
    do {                                                                   \
        if (!(cond)) { std::printf("  [FALLO] %s\n", msg); ++failures; }   \
        else         { std::printf("  [ok]    %s\n", msg); }               \
    } while (0)

int main(int argc, char** argv) {
    std::string path = argc > 1 ? argv[1] : std::string(SUSPSIM_ROOT) + "/data/ejemplo_doble_trapecio.json";
    Suspension s;
    std::string err;
    if (!loadSuspensionJson(path, s, err)) { std::printf("%s\n", err.c_str()); return 1; }
    std::printf("Modelo: %s\n", s.name.c_str());

    KinematicSolver solver(s);

    // 1) En recorrido 0 la solución debe coincidir con los puntos de entrada
    CornerState st0 = solver.solve({0.0, 0.0});
    CHECK(st0.valid, "solucion en posicion de diseno");
    CHECK(glm::length(st0.wheelCenter - s.hp.wheelCenter) < 1e-6, "centro de rueda sin moverse en travel=0");
    CHECK(glm::length(st0.tieRodOuter - s.hp.tieRodOuter) < 1e-6, "rotula de bieleta sin moverse en travel=0");

    // 2) En todo el barrido se conservan las longitudes de los sólidos rígidos
    auto sw = sweepTravel(solver, s.bc.travelMin, s.bc.travelMax, s.bc.steps, 0.0);
    bool allValid = true; double maxErr = 0;
    const double lKp = glm::length(s.hp.upperBallJoint - s.hp.lowerBallJoint);
    const double lTr = glm::length(s.hp.tieRodOuter - s.hp.tieRodInner);
    const double lWc = glm::length(s.hp.wheelCenter - s.hp.lowerBallJoint);
    for (auto& m : sw) {
        if (!m.valid) { allValid = false; continue; }
        CornerState st = solver.solve({m.travel, 0.0});
        maxErr = std::max(maxErr, std::abs(glm::length(st.upperBallJoint - st.lowerBallJoint) - lKp));
        maxErr = std::max(maxErr, std::abs(glm::length(st.tieRodOuter - st.tieRodInner) - lTr));
        maxErr = std::max(maxErr, std::abs(glm::length(st.wheelCenter - st.lowerBallJoint) - lWc));
        maxErr = std::max(maxErr, std::abs(st.wheelCenter.z - s.hp.wheelCenter.z - m.travel));
    }
    CHECK(allValid, "todas las posiciones del barrido tienen solucion");
    std::printf("          error maximo de cierre: %.2e mm\n", maxErr);
    CHECK(maxErr < 1e-5, "longitudes rigidas conservadas");

    // 3) Parámetros estáticos
    Metrics m = analyze(solver, {0.0, 0.0});
    std::printf("\nPosicion de diseno:\n");
    std::printf("  caida %.3f deg | convergencia %.3f deg | avance %.2f deg | KPI %.2f deg\n",
                m.camber, m.toe, m.caster, m.kpi);
    std::printf("  scrub %.1f mm | trail %.1f mm | h CB %.1f mm | FVSA %.0f mm | anti-dive %.1f %%\n",
                m.scrubRadius, m.mechanicalTrail, m.rollCenterHeight, m.fvsaLength, m.antiBrake);
    std::printf("  semivia %.1f mm | MR %.3f\n", m.halfTrack, m.motionRatio);
    CHECK(std::abs(m.camber + 1.0) < 0.01, "caida estatica = -1.0 deg (spindle_point del ejemplo)");
    CHECK(m.caster > 0 && m.kpi > 0, "avance y KPI positivos con el criterio de signos");

    std::printf("\nBarrido (travel, caida, conv, hCB):\n");
    for (size_t i = 0; i < sw.size(); i += 4)
        std::printf("  %7.1f  %7.3f  %7.3f  %7.1f\n", sw[i].travel, sw[i].camber, sw[i].toe, sw[i].rollCenterHeight);
    double cg = gradientAtZero(sw, &Metrics::camber) * 10;
    std::printf("  ganancia de caida %.3f deg/10mm, bump steer %.3f deg/m\n", cg,
                gradientAtZero(sw, &Metrics::toe) * 1000);
    CHECK(cg < 0, "la caida se hace mas negativa al comprimir (brazo superior mas corto)");

    // 4) Dirección: con cremallera desplazada la convergencia cambia
    Metrics ms = analyze(solver, {0.0, 10.0});
    CHECK(ms.valid && std::abs(ms.toe - m.toe) > 0.5, "la cremallera gira la rueda");

    // 5) Optimización corta: reducir bump steer moviendo la bieleta interior en Z
    Objective obj{ObjectiveType::MinBumpSteer, 0.0};
    std::vector<DesignVariable> vars = {{"tie_rod_inner", 2, -30, 30}};
    OptimizerResult r = optimize(s, vars, obj);
    std::printf("\n%s", r.message.c_str());
    CHECK(r.ok && r.finalCost < r.initialCost, "el optimizador reduce el bump steer");

    std::printf("\n%s (%d fallos)\n", failures ? "HAY FALLOS" : "TODO OK", failures);
    return failures ? 1 : 0;
}

#include "analysis/Optimizer.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <sstream>

namespace susp {

const char* objectiveName(ObjectiveType t) {
    switch (t) {
    case ObjectiveType::MinBumpSteer:           return "Minimizar bump steer";
    case ObjectiveType::TargetCamberGain:       return "Ganancia de caida objetivo [deg/10 mm]";
    case ObjectiveType::TargetRollCenter:       return "Altura de CB objetivo [mm]";
    case ObjectiveType::MinRollCenterMigration: return "Minimizar migracion del CB";
    }
    return "?";
}

double evaluateObjective(const Suspension& s, const Objective& obj, int steps) {
    constexpr double kPenalty = 1e6;
    KinematicSolver solver(s);
    auto sw = sweepTravel(solver, s.bc.travelMin, s.bc.travelMax, steps, s.bc.rack);
    for (auto& m : sw) if (!m.valid) return kPenalty;

    switch (obj.type) {
    case ObjectiveType::MinBumpSteer: {
        // RMS de la convergencia respecto a la de diseño
        double toe0 = 0; for (auto& m : sw) if (std::abs(m.travel) < 1e-9) toe0 = m.toe;
        double acc = 0; for (auto& m : sw) acc += (m.toe - toe0) * (m.toe - toe0);
        return std::sqrt(acc / sw.size());
    }
    case ObjectiveType::TargetCamberGain: {
        double g = gradientAtZero(sw, &Metrics::camber) * 10.0;
        return (g - obj.target) * (g - obj.target);
    }
    case ObjectiveType::TargetRollCenter: {
        Metrics m0 = computeGeometry(s, solver.reference());
        double e = (m0.rollCenterHeight - obj.target) / 10.0;
        return e * e;
    }
    case ObjectiveType::MinRollCenterMigration: {
        double lo = 1e300, hi = -1e300;
        for (auto& m : sw) { lo = std::min(lo, m.rollCenterHeight); hi = std::max(hi, m.rollCenterHeight); }
        return hi - lo;
    }
    }
    return kPenalty;
}

OptimizerResult optimize(const Suspension& start, const std::vector<DesignVariable>& vars,
                         const Objective& obj, const OptimizerSettings& set) {
    OptimizerResult res;
    res.best = start;
    const int n = static_cast<int>(vars.size());
    if (n == 0) { res.message = "No hay variables de diseno"; return res; }

    // Valores iniciales de cada coordenada
    std::vector<double> base(n);
    for (int i = 0; i < n; ++i) {
        Hardpoints hp = start.hp;
        Vec3* p = hardpointByName(hp, vars[i].hardpoint);
        if (!p) { res.message = "Punto desconocido: " + vars[i].hardpoint; return res; }
        base[i] = (*p)[vars[i].axis];
    }

    auto clampX = [&](std::vector<double>& x) {
        for (int i = 0; i < n; ++i) x[i] = std::clamp(x[i], vars[i].minOffset, vars[i].maxOffset);
    };
    auto build = [&](const std::vector<double>& x) {
        Suspension s = start;
        for (int i = 0; i < n; ++i) (*hardpointByName(s.hp, vars[i].hardpoint))[vars[i].axis] = base[i] + x[i];
        return s;
    };
    auto cost = [&](std::vector<double>& x) {
        clampX(x);
        ++res.evaluations;
        return evaluateObjective(build(x), obj, set.sweepSteps);
    };

    // Simplex inicial: el punto de partida + un paso en cada dirección
    std::vector<std::vector<double>> P(n + 1, std::vector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        double step = 0.25 * (vars[i].maxOffset - vars[i].minOffset);
        P[i + 1][i] = (std::abs(step) > 1e-9) ? step : 1.0;
    }
    std::vector<double> F(n + 1);
    for (int i = 0; i <= n; ++i) F[i] = cost(P[i]);
    res.initialCost = F[0];

    const double a = 1.0, g = 2.0, r = 0.5, sh = 0.5;  // reflexión, expansión, contracción, encogimiento
    std::vector<int> idx(n + 1);
    for (res.iterations = 0; res.iterations < set.maxIterations; ++res.iterations) {
        std::iota(idx.begin(), idx.end(), 0);
        std::sort(idx.begin(), idx.end(), [&](int i, int j) { return F[i] < F[j]; });
        int best = idx[0], worst = idx[n], second = idx[n - 1];
        res.history.push_back(F[best]);
        if (std::abs(F[worst] - F[best]) < set.tolerance) break;

        // Centroide sin el peor
        std::vector<double> c(n, 0.0);
        for (int k = 0; k < n; ++k)  // idx[0..n-1] = todos menos el peor
            for (int i = 0; i < n; ++i) c[i] += P[idx[k]][i] / n;

        auto lerp = [&](double t) {  // c + t (c - peor)
            std::vector<double> x(n);
            for (int i = 0; i < n; ++i) x[i] = c[i] + t * (c[i] - P[worst][i]);
            return x;
        };
        std::vector<double> xr = lerp(a);
        double fr = cost(xr);
        if (fr < F[best]) {
            std::vector<double> xe = lerp(g);
            double fe = cost(xe);
            if (fe < fr) { P[worst] = xe; F[worst] = fe; } else { P[worst] = xr; F[worst] = fr; }
        } else if (fr < F[second]) {
            P[worst] = xr; F[worst] = fr;
        } else {
            std::vector<double> xc = lerp(fr < F[worst] ? r : -r);
            double fc = cost(xc);
            if (fc < std::min(fr, F[worst])) { P[worst] = xc; F[worst] = fc; }
            else {
                for (int k = 1; k <= n; ++k) {
                    int j = idx[k];
                    for (int i = 0; i < n; ++i) P[j][i] = P[best][i] + sh * (P[j][i] - P[best][i]);
                    F[j] = cost(P[j]);
                }
            }
        }
    }

    int bestI = static_cast<int>(std::min_element(F.begin(), F.end()) - F.begin());
    res.offsets = P[bestI];
    res.finalCost = F[bestI];
    res.best = build(P[bestI]);
    res.ok = res.finalCost <= res.initialCost;

    std::ostringstream msg;
    msg << "Objetivo: " << objectiveName(obj.type) << "\n"
        << "Coste inicial " << res.initialCost << " -> final " << res.finalCost
        << " (" << res.iterations << " iteraciones, " << res.evaluations << " evaluaciones)\n";
    const char* ax = "XYZ";
    for (int i = 0; i < n; ++i)
        msg << "- " << vars[i].hardpoint << "." << ax[vars[i].axis] << ": "
            << (res.offsets[i] >= 0 ? "+" : "") << res.offsets[i] << " mm\n";
    res.message = msg.str();
    return res;
}

} // namespace susp

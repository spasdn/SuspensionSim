#include "analysis/Analysis.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace susp {
namespace {

constexpr double kRad2Deg = 57.29577951308232;
constexpr double kInf = std::numeric_limits<double>::infinity();

// Plano por 3 puntos -> normal unitaria n y constante c (n·p = c)
struct Plane { Vec3 n; double c; };
Plane planeFrom(const Vec3& a, const Vec3& b, const Vec3& p) {
    Vec3 n = glm::normalize(glm::cross(b - a, p - a));
    return {n, glm::dot(n, a)};
}

// Corta dos planos de brazo con un plano de vista (eje fijo 'fixedAxis' = valor).
// Devuelve el punto 2D (u,v) de corte de las dos rectas resultantes, o "infinito"
// si son paralelas; en ese caso 'dir' es la dirección común (para construcciones).
struct Line2DResult { bool atInfinity; double u, v; double du, dv; };
Line2DResult intersectInView(const Plane& p1, const Plane& p2, int fixedAxis, double value,
                             int uAxis, int vAxis) {
    double a1 = p1.n[uAxis], b1 = p1.n[vAxis], c1 = p1.c - p1.n[fixedAxis] * value;
    double a2 = p2.n[uAxis], b2 = p2.n[vAxis], c2 = p2.c - p2.n[fixedAxis] * value;
    double det = a1 * b2 - b1 * a2;
    Line2DResult r{};
    r.du = b1; r.dv = -a1;   // dirección de la recta del brazo 1 en esa vista
    if (std::abs(det) < 1e-9) { r.atInfinity = true; return r; }
    r.atInfinity = false;
    r.u = (c1 * b2 - b1 * c2) / det;
    r.v = (a1 * c2 - c1 * a2) / det;
    return r;
}

} // namespace

Metrics computeGeometry(const Suspension& s, const CornerState& st) {
    Metrics m;
    if (!st.valid) return m;
    m.valid = true;
    const double side = s.sideSign();
    const Hardpoints& h = s.hp;
    const Vec3& wc = st.wheelCenter;
    const Vec3& cp = st.contactPatch;
    m.contactPatch = cp;

    // ---------------- Caída y convergencia (eje de mangueta hacia fuera) ----------
    Vec3 a = glm::normalize(st.spindlePoint - wc);
    m.camber = -kRad2Deg * std::atan2(a.z, std::sqrt(a.x * a.x + a.y * a.y));
    m.toe    =  kRad2Deg * std::atan2(a.x, side * a.y);

    // ---------------- Eje de pivote ---------------------------------------------
    Vec3 k = glm::normalize(st.upperBallJoint - st.lowerBallJoint);
    m.caster = kRad2Deg * std::atan2(-k.x, k.z);
    m.kpi    = kRad2Deg * std::atan2(-side * k.y, k.z);

    // Corte del eje con el suelo (plano z = z del contacto)
    double tG = (cp.z - st.lowerBallJoint.z) / k.z;
    Vec3 G = st.lowerBallJoint + tG * k;
    m.scrubRadius     = side * (cp.y - G.y);
    m.mechanicalTrail = G.x - cp.x;
    double tW = (wc.z - st.lowerBallJoint.z) / k.z;
    Vec3 K = st.lowerBallJoint + tW * k;
    m.kingpinOffsetWC = side * (wc.y - K.y);

    // ---------------- Centros instantáneos ---------------------------------------
    Plane lower = planeFrom(h.lcaFront, h.lcaRear, st.lowerBallJoint);
    Plane upper = planeFrom(h.ucaFront, h.ucaRear, st.upperBallJoint);

    // Vista frontal: plano x = x del centro de rueda, coordenadas (y, z)
    auto fv = intersectInView(lower, upper, 0, wc.x, 1, 2);
    double dirY, dirZ;  // dirección de la recta contacto -> CI
    if (fv.atInfinity) {
        m.fvicAtInfinity = true;
        m.fvsaLength = kInf;
        dirY = fv.du; dirZ = fv.dv;
    } else {
        m.fvic = Vec3(wc.x, fv.u, fv.v);
        m.fvsaLength = std::abs(fv.u - cp.y);
        dirY = fv.u - cp.y; dirZ = fv.v - cp.z;
    }
    // Centro de balanceo: corte de la recta contacto-CI con el plano de simetría y = 0
    if (std::abs(dirY) > 1e-12) {
        double zrc = cp.z + (0.0 - cp.y) * dirZ / dirY;
        m.rollCenter = Vec3(cp.x, 0.0, zrc);
        m.rollCenterHeight = zrc - cp.z;
    }

    // Vista lateral: plano y = y del centro de rueda, coordenadas (x, z)
    auto sv = intersectInView(lower, upper, 1, wc.y, 0, 2);
    // Sentido "hacia el centro del coche": hacia atrás en el eje delantero, hacia delante en el trasero
    const double inward = (s.axle == Axle::Front) ? -1.0 : 1.0;
    auto tanAngleFrom = [&](const Vec3& origin) {
        if (sv.atInfinity) {
            double dx = sv.du * inward;  // recta paralela a los brazos
            return std::abs(dx) < 1e-12 ? 0.0 : sv.dv / dx;
        }
        double dx = (sv.u - origin.x) * inward;
        return std::abs(dx) < 1e-12 ? 0.0 : (sv.v - origin.z) / dx;
    };
    if (sv.atInfinity) {
        m.svicAtInfinity = true;
        m.svsaLength = kInf;
    } else {
        m.svic = Vec3(sv.u, wc.y, sv.v);
        m.svsaLength = std::abs(sv.u - cp.x);
    }
    const VehicleParams& v = s.vehicle;
    const double LoverH = v.wheelbase / v.cgHeight;
    const double brakeFrac = (s.axle == Axle::Front) ? v.frontBrakeBias : 1.0 - v.frontBrakeBias;
    const double driveFrac = (s.axle == Axle::Front) ? v.frontDriveFraction : 1.0 - v.frontDriveFraction;
    // Frenos exteriores: la fuerza actúa en el contacto. Transmisión interior: en el centro de rueda.
    m.antiBrake = 100.0 * tanAngleFrom(cp) * LoverH * brakeFrac;
    m.antiAccel = 100.0 * tanAngleFrom(wc) * LoverH * driveFrac;

    // ---------------- Posición de rueda -----------------------------------------
    m.halfTrack    = side * cp.y;
    m.wheelCenterX = wc.x;

    // ---------------- Amortiguador ----------------------------------------------
    if (h.hasDamper) m.damperLength = glm::length(h.damperChassis - st.damperArm);
    return m;
}

Metrics analyze(KinematicSolver& solver, const KinematicInputs& in) {
    CornerState st = solver.solve(in);
    Metrics m = computeGeometry(solver.suspension(), st);
    m.travel = in.travel;
    m.rack = in.rack;
    if (!m.valid) return m;

    if (solver.suspension().hp.hasDamper) {
        const double d = 0.5;  // mm
        Metrics up = computeGeometry(solver.suspension(), solver.solve({in.travel + d, in.rack}));
        Metrics dn = computeGeometry(solver.suspension(), solver.solve({in.travel - d, in.rack}));
        if (up.valid && dn.valid)
            m.motionRatio = -(up.damperLength - dn.damperLength) / (2 * d);  // + si comprime al subir la rueda
        solver.solve(in);  // dejar el arranque en caliente en la posición pedida
    }
    return m;
}

std::vector<Metrics> sweepTravel(KinematicSolver& solver, double tMin, double tMax, int steps,
                                 double rack) {
    steps = std::max(steps, 3);
    std::vector<Metrics> out(steps);
    std::vector<double> t(steps);
    for (int i = 0; i < steps; ++i) t[i] = tMin + (tMax - tMin) * i / (steps - 1);

    // Desde la posición más cercana a 0 hacia fuera, para que el arranque en caliente funcione
    int i0 = 0;
    for (int i = 1; i < steps; ++i)
        if (std::abs(t[i]) < std::abs(t[i0])) i0 = i;

    solver.resetWarmStart();
    for (int i = i0; i < steps; ++i) out[i] = analyze(solver, {t[i], rack});
    solver.resetWarmStart();
    for (int i = i0 - 1; i >= 0; --i) out[i] = analyze(solver, {t[i], rack});
    solver.resetWarmStart();
    return out;
}

double gradientAtZero(const std::vector<Metrics>& sw, double Metrics::*field) {
    if (sw.size() < 3) return 0.0;
    size_t i0 = 0;
    for (size_t i = 1; i < sw.size(); ++i)
        if (std::abs(sw[i].travel) < std::abs(sw[i0].travel)) i0 = i;
    size_t a = (i0 == 0) ? 0 : i0 - 1;
    size_t b = (i0 + 1 >= sw.size()) ? sw.size() - 1 : i0 + 1;
    if (!sw[a].valid || !sw[b].valid || sw[b].travel == sw[a].travel) return 0.0;
    return (sw[b].*field - sw[a].*field) / (sw[b].travel - sw[a].travel);
}

} // namespace susp

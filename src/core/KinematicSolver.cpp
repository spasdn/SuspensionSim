#include "core/KinematicSolver.h"

#include <algorithm>
#include <cmath>
#include <optional>

namespace susp {
namespace {

// Base ortonormal a partir de 3 puntos (columna 0 = p0->p1)
glm::dmat3 frameFrom(const Vec3& p0, const Vec3& p1, const Vec3& p2) {
    Vec3 e1 = glm::normalize(p1 - p0);
    Vec3 v  = p2 - p0;
    Vec3 e2 = glm::normalize(v - glm::dot(v, e1) * e1);
    Vec3 e3 = glm::cross(e1, e2);
    return glm::dmat3(e1, e2, e3);
}

// Intersección de 3 esferas (trilateración). Devuelve la solución más cercana a 'near'.
std::optional<Vec3> trilaterate(const Vec3& p1, double r1, const Vec3& p2, double r2,
                                const Vec3& p3, double r3, const Vec3& near) {
    const double d = glm::length(p2 - p1);
    if (d < 1e-9) return std::nullopt;
    Vec3 ex = (p2 - p1) / d;
    double i = glm::dot(ex, p3 - p1);
    Vec3 tmp = p3 - p1 - i * ex;
    double tl = glm::length(tmp);
    if (tl < 1e-9) return std::nullopt;  // centros alineados
    Vec3 ey = tmp / tl;
    Vec3 ez = glm::cross(ex, ey);
    double j = glm::dot(ey, p3 - p1);

    double x = (r1 * r1 - r2 * r2 + d * d) / (2 * d);
    double y = (r1 * r1 - r3 * r3 + i * i + j * j) / (2 * j) - (i / j) * x;
    double z2 = r1 * r1 - x * x - y * y;
    if (z2 < -1e-6 * r1 * r1) return std::nullopt;  // las esferas no se cortan
    double z = std::sqrt(std::max(0.0, z2));

    Vec3 a = p1 + x * ex + y * ey + z * ez;
    Vec3 b = p1 + x * ex + y * ey - z * ez;
    return glm::length(a - near) < glm::length(b - near) ? a : b;
}

} // namespace

Vec3 KinematicSolver::rotateAboutAxis(const Vec3& p, const Vec3& a, const Vec3& b, double ang) {
    // Fórmula de Rodrigues: gira p alrededor del eje que pasa por a con dirección b-a
    Vec3 k = glm::normalize(b - a);
    Vec3 v = p - a;
    double c = std::cos(ang), s = std::sin(ang);
    Vec3 r = v * c + glm::cross(k, v) * s + k * glm::dot(k, v) * (1.0 - c);
    return a + r;
}

Vec3 KinematicSolver::contactPatchFrom(const Vec3& wc, const Vec3& sp, double radius) {
    // Neumático rígido: punto más bajo de la circunferencia de la rueda.
    Vec3 axis = glm::normalize(sp - wc);
    Vec3 down(0, 0, -1);
    Vec3 inPlane = down - glm::dot(down, axis) * axis;  // "abajo" proyectado en el plano de rueda
    return wc + radius * glm::normalize(inPlane);
}

KinematicSolver::KinematicSolver(const Suspension& s) : s_(s) {
    const Hardpoints& h = s_.hp;
    lKingpin_ = glm::length(h.upperBallJoint - h.lowerBallJoint);
    lLbjTro_  = glm::length(h.tieRodOuter - h.lowerBallJoint);
    lUbjTro_  = glm::length(h.tieRodOuter - h.upperBallJoint);
    lTieRod_  = glm::length(h.tieRodOuter - h.tieRodInner);

    ref_.valid = true;
    ref_.lowerBallJoint = h.lowerBallJoint;
    ref_.upperBallJoint = h.upperBallJoint;
    ref_.tieRodOuter    = h.tieRodOuter;
    ref_.tieRodInner    = h.tieRodInner;
    ref_.wheelCenter    = h.wheelCenter;
    ref_.spindlePoint   = h.spindlePoint;
    ref_.contactPatch   = contactPatchFrom(h.wheelCenter, h.spindlePoint, s_.tire.radius);
    ref_.damperArm      = h.damperArm;
    last_ = ref_;
}

CornerState KinematicSolver::solveAtLowerAngle(double theta, double rack,
                                               const CornerState* guess) const {
    const Hardpoints& h = s_.hp;
    CornerState st;
    st.lowerAngle = theta;

    // 1) Rótula inferior: gira con el brazo inferior
    st.lowerBallJoint = rotateAboutAxis(h.lowerBallJoint, h.lcaFront, h.lcaRear, theta);

    // 2) Rótula superior: buscar φ tal que |UBJ - LBJ| = longitud del pivote (Newton 1D)
    auto f = [&](double phi) {
        Vec3 u = rotateAboutAxis(h.upperBallJoint, h.ucaFront, h.ucaRear, phi);
        return glm::length(u - st.lowerBallJoint) - lKingpin_;
    };
    double phi = guess ? guess->upperAngle : 0.0;
    bool ok = false;
    for (int it = 0; it < 50; ++it) {
        double fx = f(phi);
        if (std::abs(fx) < 1e-9) { ok = true; break; }
        const double hstep = 1e-7;
        double df = (f(phi + hstep) - f(phi - hstep)) / (2 * hstep);
        if (std::abs(df) < 1e-12) break;
        phi -= fx / df;
    }
    if (!ok) return st;  // valid = false
    st.upperAngle = phi;
    st.upperBallJoint = rotateAboutAxis(h.upperBallJoint, h.ucaFront, h.ucaRear, phi);

    // 3) Rótula de la bieleta: corte de tres esferas
    st.tieRodInner = h.tieRodInner + Vec3(0, rack, 0);
    Vec3 near = guess ? guess->tieRodOuter : h.tieRodOuter;
    auto tro = trilaterate(st.lowerBallJoint, lLbjTro_, st.upperBallJoint, lUbjTro_,
                           st.tieRodInner, lTieRod_, near);
    if (!tro) return st;
    st.tieRodOuter = *tro;

    // 4) Transformación rígida de la mangueta: R * p + t
    glm::dmat3 F0 = frameFrom(h.lowerBallJoint, h.upperBallJoint, h.tieRodOuter);
    glm::dmat3 F1 = frameFrom(st.lowerBallJoint, st.upperBallJoint, st.tieRodOuter);
    glm::dmat3 R = F1 * glm::transpose(F0);
    auto move = [&](const Vec3& p) { return st.lowerBallJoint + R * (p - h.lowerBallJoint); };

    st.wheelCenter  = move(h.wheelCenter);
    st.spindlePoint = move(h.spindlePoint);
    st.contactPatch = contactPatchFrom(st.wheelCenter, st.spindlePoint, s_.tire.radius);

    // 5) Punto del amortiguador según a qué pieza va unido
    if (h.hasDamper) {
        switch (h.damperAttach) {
        case Attach::LowerArm: st.damperArm = rotateAboutAxis(h.damperArm, h.lcaFront, h.lcaRear, theta); break;
        case Attach::UpperArm: st.damperArm = rotateAboutAxis(h.damperArm, h.ucaFront, h.ucaRear, phi);   break;
        case Attach::Upright:  st.damperArm = move(h.damperArm); break;
        }
    }
    st.valid = true;
    return st;
}

CornerState KinematicSolver::solve(const KinematicInputs& in) {
    const double target = ref_.wheelCenter.z + in.travel;
    double theta = last_.valid ? last_.lowerAngle : 0.0;
    CornerState guess = last_.valid ? last_ : ref_;

    // Newton sobre θ para que z(centro de rueda) = objetivo
    for (int it = 0; it < 60; ++it) {
        CornerState st = solveAtLowerAngle(theta, in.rack, &guess);
        if (!st.valid) break;
        double g = st.wheelCenter.z - target;
        if (std::abs(g) < 1e-7) { last_ = st; return st; }

        const double hstep = 1e-6;
        CornerState a = solveAtLowerAngle(theta + hstep, in.rack, &st);
        CornerState b = solveAtLowerAngle(theta - hstep, in.rack, &st);
        if (!a.valid || !b.valid) break;
        double dg = (a.wheelCenter.z - b.wheelCenter.z) / (2 * hstep);
        if (std::abs(dg) < 1e-9) break;

        double step = g / dg;
        step = std::clamp(step, -0.2, 0.2);  // limitar el paso (rad) para no saltar de rama
        theta -= step;
        guess = st;
    }
    CornerState fail;
    fail.valid = false;
    return fail;
}

} // namespace susp

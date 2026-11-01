#include "render/SceneBuilder.h"

#include <cmath>

using susp::Vec3;

namespace {

// Dibuja una esquina completa. 'mirror' refleja en Y (lado opuesto); 'dim' atenúa colores.
void drawCorner(Renderer& r, const susp::Suspension& s, const susp::CornerState& st,
                bool mirror, float dim, const SceneOptions& opt) {
    if (!st.valid) return;
    const susp::Hardpoints& h = s.hp;
    auto P = [&](const Vec3& v) { return glm::vec3(v.x, mirror ? -v.y : v.y, v.z); };
    auto C = [&](Color c, float a = 1.0f) { return Color{c.r * dim, c.g * dim, c.b * dim, a}; };

    // Trapecio inferior (azul) y superior (verde): triángulo semitransparente + aristas
    r.addTriangle(P(h.lcaFront), P(h.lcaRear), P(st.lowerBallJoint), C(colors::Blue, 0.35f));
    r.addLine(P(h.lcaFront), P(st.lowerBallJoint), C(colors::Blue));
    r.addLine(P(h.lcaRear),  P(st.lowerBallJoint), C(colors::Blue));
    r.addTriangle(P(h.ucaFront), P(h.ucaRear), P(st.upperBallJoint), C(colors::Green, 0.35f));
    r.addLine(P(h.ucaFront), P(st.upperBallJoint), C(colors::Green));
    r.addLine(P(h.ucaRear),  P(st.upperBallJoint), C(colors::Green));

    // Bieleta de dirección (naranja)
    r.addLine(P(st.tieRodInner), P(st.tieRodOuter), C(colors::Orange));

    // Mangueta: triángulo rótula inf - rótula sup - bieleta, y brazo al centro de rueda
    r.addTriangle(P(st.lowerBallJoint), P(st.upperBallJoint), P(st.tieRodOuter), C(colors::Gray));
    r.addLine(P(st.lowerBallJoint), P(st.wheelCenter), C(colors::Gray));
    r.addLine(P(st.upperBallJoint), P(st.wheelCenter), C(colors::Gray));

    // Rueda: dos circunferencias (caras del neumático) unidas por segmentos
    glm::vec3 wc = P(st.wheelCenter);
    glm::vec3 axis = glm::normalize(P(st.spindlePoint) - wc);
    float R = static_cast<float>(s.tire.radius), W = static_cast<float>(s.tire.width);
    glm::vec3 in = wc - axis * (W * 0.5f), out = wc + axis * (W * 0.5f);
    r.addCircle(in, axis, R, C(colors::White));
    r.addCircle(out, axis, R, C(colors::White));
    r.addCircle(wc, axis, R * 0.55f, C(colors::Dark));  // llanta
    {
        glm::vec3 helper = std::abs(axis.z) < 0.9f ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
        glm::vec3 u = glm::normalize(glm::cross(axis, helper)), v = glm::cross(axis, u);
        for (int i = 0; i < 12; ++i) {
            float a = 2.0f * 3.14159265f * i / 12;
            glm::vec3 d = R * (std::cos(a) * u + std::sin(a) * v);
            r.addLine(in + d, out + d, C(colors::Dark));
        }
    }
    r.addLine(wc, P(st.spindlePoint), C(colors::Cyan));  // eje de mangueta (hacia fuera)
    r.addPoint(P(st.contactPatch), C(colors::Red));

    // Muelle-amortiguador (rojo)
    if (h.hasDamper) r.addLine(P(h.damperChassis), P(st.damperArm), C(colors::Red));

    // Eje de pivote prolongado hasta el suelo (amarillo discontinuo)
    if (opt.showKingpinAxis) {
        Vec3 k = st.upperBallJoint - st.lowerBallJoint;
        double t = (st.contactPatch.z - st.lowerBallJoint.z) / k.z;
        r.addDashedLine(P(st.upperBallJoint + 0.2 * k), P(st.lowerBallJoint + t * k), C(colors::Yellow), 15.0f);
    }

    // Puntos duros
    if (opt.showHardpoints) {
        for (const Vec3& p : {h.lcaFront, h.lcaRear, h.ucaFront, h.ucaRear})
            r.addPoint(P(p), C(colors::White));
        r.addPoint(P(st.tieRodInner), C(colors::Orange));
        for (const Vec3& p : {st.lowerBallJoint, st.upperBallJoint, st.tieRodOuter, st.wheelCenter})
            r.addPoint(P(p), C(colors::Yellow));
    }
}

} // namespace

void buildScene(Renderer& r, const susp::Suspension& s, const susp::CornerState& st,
                const susp::CornerState& ref, const susp::Metrics& m,
                const susp::Mesh* chassis, const SceneOptions& opt) {
    const float groundZ = static_cast<float>(ref.contactPatch.z);
    if (opt.showGrid) r.addGrid(groundZ, 2000.0f, 100.0f, Color{0.25f, 0.27f, 0.30f, 1.0f});
    if (opt.showAxes) r.addAxes(glm::vec3(0, 0, groundZ), 300.0f);

    // Chasis importado (.obj): caras muy transparentes + aristas
    if (opt.showChassis && chassis && !chassis->empty()) {
        Color face = colors::Gray.withAlpha(opt.chassisAlpha);
        for (auto& t : chassis->triangles)
            r.addTriangle(chassis->vertices[t[0]], chassis->vertices[t[1]], chassis->vertices[t[2]], face);
        for (auto& e : chassis->edges)
            r.addLine(chassis->vertices[e[0]], chassis->vertices[e[1]], colors::Gray.withAlpha(0.6f));
    }

    if (opt.showReference) drawCorner(r, s, ref, false, 0.35f, opt);
    drawCorner(r, s, st, false, 1.0f, opt);
    if (opt.showMirror) drawCorner(r, s, st, true, 0.6f, opt);

    // Construcción del centro de balanceo (vista frontal): magenta
    if (opt.showFrontViewIC && m.valid) {
        glm::vec3 cp(m.contactPatch);
        glm::vec3 rc(m.rollCenter);
        if (!m.fvicAtInfinity) {
            glm::vec3 ic(m.fvic);
            r.addDashedLine(glm::vec3(st.lowerBallJoint), ic, colors::Magenta.withAlpha(0.7f));
            r.addDashedLine(glm::vec3(st.upperBallJoint), ic, colors::Magenta.withAlpha(0.7f));
            r.addDashedLine(cp, ic, colors::Magenta);
            r.addPoint(ic, colors::Magenta);
        }
        r.addLine(cp, rc, colors::Magenta);
        r.addCross(rc, 25.0f, colors::Magenta);
    }
    // Centro instantáneo lateral (anti-dive / anti-squat): cian
    if (opt.showSideViewIC && m.valid && !m.svicAtInfinity) {
        glm::vec3 ic(m.svic);
        r.addDashedLine(glm::vec3(m.contactPatch), ic, colors::Cyan);
        r.addPoint(ic, colors::Cyan);
    }
}

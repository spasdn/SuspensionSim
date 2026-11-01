#pragma once
// =============================================================================
// SceneBuilder.h — Traduce el modelo (puntos, estado, resultados) a líneas y
// triángulos. Es el ÚNICO sitio que sabe "cómo se dibuja una suspensión".
// Si quieres dibujar algo nuevo, añádelo aquí (ver docs/OpenGL/Cómo dibujar algo nuevo.md)
// =============================================================================
#include "analysis/Analysis.h"
#include "io/ObjLoader.h"
#include "render/Renderer.h"

struct SceneOptions {
    bool showGrid = true;
    bool showAxes = true;
    bool showMirror = true;        // dibujar también el lado opuesto (simetría)
    bool showReference = false;    // dibujar en gris la posición de diseño
    bool showChassis = true;
    bool showKingpinAxis = true;
    bool showFrontViewIC = true;   // construcción del centro de balanceo
    bool showSideViewIC = false;
    bool showHardpoints = true;
    float chassisAlpha = 0.18f;
};

void buildScene(Renderer& r, const susp::Suspension& s, const susp::CornerState& st,
                const susp::CornerState& reference, const susp::Metrics& m,
                const susp::Mesh* chassis, const SceneOptions& opt);

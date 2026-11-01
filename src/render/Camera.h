#pragma once
// =============================================================================
// Camera.h — Cámara orbital con Z hacia arriba (como en CAD)
// -----------------------------------------------------------------------------
// La cámara gira alrededor de un punto 'target'. Produce dos matrices:
//   view       : mueve el mundo para que la cámara quede en el origen mirando a -Z
//   projection : aplica la perspectiva (lo lejano se ve más pequeño)
// En el shader: posición_en_pantalla = projection * view * posición_en_mundo
// =============================================================================
#include <glm/glm.hpp>

class Camera {
public:
    glm::vec3 target{0.0f, 0.0f, 250.0f};  // punto al que se mira [mm]
    float distance = 3000.0f;               // distancia al objetivo [mm]
    float yawDeg   = 35.0f;                 // giro alrededor de Z
    float pitchDeg = 20.0f;                 // elevación sobre el plano XY
    float fovDeg   = 40.0f;
    bool  orthographic = false;             // proyección ortogonal (vistas tipo plano)

    glm::vec3 position() const;
    glm::mat4 view() const;
    glm::mat4 projection(float aspect) const;

    // Controles de ratón (dx, dy en píxeles)
    void orbit(float dx, float dy);
    void pan(float dx, float dy, float viewportHeight);
    void zoom(float wheelSteps);

    // Vistas predefinidas
    void viewFront();  // mirando hacia atrás desde delante (plano YZ)
    void viewSide();   // desde la izquierda (plano XZ)
    void viewTop();    // planta (plano XY)
    void viewIso();
};

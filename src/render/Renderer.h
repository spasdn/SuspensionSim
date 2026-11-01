#pragma once
// =============================================================================
// Renderer.h — Dibujo "inmediato" de líneas, triángulos y puntos
// -----------------------------------------------------------------------------
// Uso típico en cada fotograma:
//     renderer.begin();
//     renderer.addLine(a, b, color);
//     renderer.addTriangle(a, b, c, color);
//     renderer.flush(camara.projection(aspect) * camara.view());
//
// Por dentro: acumula vértices en listas de la CPU y en flush() los copia a la
// GPU (VBO) y los dibuja con una sola llamada por tipo. Para la cantidad de
// geometría de una suspensión (cientos de elementos) es más que suficiente.
// =============================================================================
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "render/Shader.h"

struct Color {
    float r, g, b, a = 1.0f;
    Color withAlpha(float alpha) const { return {r, g, b, alpha}; }
};

namespace colors {
inline constexpr Color White   {0.92f, 0.92f, 0.92f};
inline constexpr Color Gray    {0.55f, 0.57f, 0.60f};
inline constexpr Color Dark    {0.30f, 0.32f, 0.35f};
inline constexpr Color Red     {0.90f, 0.25f, 0.22f};
inline constexpr Color Green   {0.30f, 0.80f, 0.35f};
inline constexpr Color Blue    {0.25f, 0.50f, 0.95f};
inline constexpr Color Orange  {1.00f, 0.60f, 0.15f};
inline constexpr Color Yellow  {0.95f, 0.85f, 0.20f};
inline constexpr Color Magenta {0.90f, 0.30f, 0.85f};
inline constexpr Color Cyan    {0.25f, 0.85f, 0.90f};
} // namespace colors

class Renderer {
public:
    bool init(std::string& error);
    void shutdown();

    void begin();  // vacía las listas

    // --- Primitivas básicas -------------------------------------------------
    void addLine(const glm::vec3& a, const glm::vec3& b, const Color& c);
    void addTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const Color& col);
    void addPoint(const glm::vec3& p, const Color& c);

    // --- Ayudas construidas con las primitivas --------------------------------
    void addDashedLine(const glm::vec3& a, const glm::vec3& b, const Color& c, float dash = 20.0f);
    void addCircle(const glm::vec3& center, const glm::vec3& normal, float radius, const Color& c, int segments = 48);
    void addCross(const glm::vec3& p, float size, const Color& c);
    void addGrid(float z, float halfSize, float spacing, const Color& c);
    void addAxes(const glm::vec3& origin, float length);

    // Dibuja todo lo acumulado con la matriz cámara (projection * view)
    void flush(const glm::mat4& viewProj, float pointSize = 8.0f);

private:
    struct Vertex { glm::vec3 pos; glm::vec4 color; };
    void draw(const std::vector<Vertex>& v, GLenum mode);

    Shader shader_;
    GLuint vao_ = 0, vbo_ = 0;
    std::vector<Vertex> lines_, opaqueTris_, transparentTris_, points_;
};

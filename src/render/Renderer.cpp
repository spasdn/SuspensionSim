#include "render/Renderer.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace {
// GLSL 3.30 core. 'layout(location = N)' enlaza cada atributo con glVertexAttribPointer(N, ...)
const char* kVertexShader = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
uniform mat4 uViewProj;
uniform float uPointSize;
out vec4 vColor;
void main() {
    gl_Position  = uViewProj * vec4(aPos, 1.0);
    gl_PointSize = uPointSize;
    vColor = aColor;
}
)";

const char* kFragmentShader = R"(
#version 330 core
in vec4 vColor;
out vec4 FragColor;
void main() {
    FragColor = vColor;
}
)";
} // namespace

bool Renderer::init(std::string& error) {
    if (!shader_.compile(kVertexShader, kFragmentShader, error)) return false;

    // VAO = "receta" de cómo leer los vértices; VBO = el buffer con los datos en la GPU
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    // atributo 0: posición (3 floats)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    // atributo 1: color (4 floats)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glBindVertexArray(0);
    return true;
}

void Renderer::shutdown() {
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
    shader_.destroy();
    vbo_ = vao_ = 0;
}

void Renderer::begin() {
    lines_.clear();
    opaqueTris_.clear();
    transparentTris_.clear();
    points_.clear();
}

void Renderer::addLine(const glm::vec3& a, const glm::vec3& b, const Color& c) {
    glm::vec4 col(c.r, c.g, c.b, c.a);
    lines_.push_back({a, col});
    lines_.push_back({b, col});
}

void Renderer::addTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const Color& col) {
    // Sombreado plano muy simple calculado en la CPU: más claro si la cara mira a la "luz"
    glm::vec3 n = glm::cross(b - a, c - a);
    float len = glm::length(n);
    float shade = 1.0f;
    if (len > 1e-6f) {
        const glm::vec3 light = glm::normalize(glm::vec3(0.4f, 0.3f, 1.0f));
        shade = 0.55f + 0.45f * std::abs(glm::dot(n / len, light));
    }
    glm::vec4 v(col.r * shade, col.g * shade, col.b * shade, col.a);
    auto& list = col.a < 0.999f ? transparentTris_ : opaqueTris_;
    list.push_back({a, v});
    list.push_back({b, v});
    list.push_back({c, v});
}

void Renderer::addPoint(const glm::vec3& p, const Color& c) {
    points_.push_back({p, glm::vec4(c.r, c.g, c.b, c.a)});
}

void Renderer::addDashedLine(const glm::vec3& a, const glm::vec3& b, const Color& c, float dash) {
    float len = glm::length(b - a);
    if (len < 1e-6f) return;
    glm::vec3 d = (b - a) / len;
    for (float t = 0.0f; t < len; t += 2.0f * dash)
        addLine(a + d * t, a + d * std::min(t + dash, len), c);
}

void Renderer::addCircle(const glm::vec3& center, const glm::vec3& normal, float radius, const Color& c, int seg) {
    glm::vec3 n = glm::normalize(normal);
    glm::vec3 helper = std::abs(n.z) < 0.9f ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
    glm::vec3 u = glm::normalize(glm::cross(n, helper));
    glm::vec3 v = glm::cross(n, u);
    glm::vec3 prev = center + radius * u;
    for (int i = 1; i <= seg; ++i) {
        float a = 2.0f * 3.14159265f * i / seg;
        glm::vec3 p = center + radius * (std::cos(a) * u + std::sin(a) * v);
        addLine(prev, p, c);
        prev = p;
    }
}

void Renderer::addCross(const glm::vec3& p, float s, const Color& c) {
    addLine(p - glm::vec3(s, 0, 0), p + glm::vec3(s, 0, 0), c);
    addLine(p - glm::vec3(0, s, 0), p + glm::vec3(0, s, 0), c);
    addLine(p - glm::vec3(0, 0, s), p + glm::vec3(0, 0, s), c);
}

void Renderer::addGrid(float z, float half, float spacing, const Color& c) {
    for (float t = -half; t <= half + 0.5f; t += spacing) {
        addLine({t, -half, z}, {t, half, z}, c);
        addLine({-half, t, z}, {half, t, z}, c);
    }
}

void Renderer::addAxes(const glm::vec3& o, float L) {
    addLine(o, o + glm::vec3(L, 0, 0), colors::Red);    // X adelante
    addLine(o, o + glm::vec3(0, L, 0), colors::Green);  // Y izquierda
    addLine(o, o + glm::vec3(0, 0, L), colors::Blue);   // Z arriba
}

void Renderer::draw(const std::vector<Vertex>& v, GLenum mode) {
    if (v.empty()) return;
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(Vertex), v.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(mode, 0, static_cast<GLsizei>(v.size()));
}

void Renderer::flush(const glm::mat4& viewProj, float pointSize) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);

    shader_.use();
    shader_.setMat4("uViewProj", viewProj);
    shader_.setFloat("uPointSize", pointSize);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    // 1) Sólidos opacos  2) líneas  3) puntos  4) transparentes sin escribir profundidad
    draw(opaqueTris_, GL_TRIANGLES);
    draw(lines_, GL_LINES);
    draw(points_, GL_POINTS);
    glDepthMask(GL_FALSE);
    draw(transparentTris_, GL_TRIANGLES);
    glDepthMask(GL_TRUE);

    glBindVertexArray(0);
}

#pragma once
// =============================================================================
// Shader.h — Un "programa" de GPU: vertex shader + fragment shader
// -----------------------------------------------------------------------------
// - El VERTEX shader se ejecuta una vez por vértice: decide DÓNDE cae en pantalla.
// - El FRAGMENT shader se ejecuta una vez por píxel: decide DE QUÉ COLOR es.
// Ambos se escriben en GLSL (un lenguaje parecido a C) y se compilan en tiempo
// de ejecución. Ver docs/OpenGL/OpenGL desde cero.md
// =============================================================================
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    bool compile(const char* vertexSrc, const char* fragmentSrc, std::string& error);
    void destroy();
    void use() const { glUseProgram(id_); }

    // Pasar valores "uniform" (constantes para toda la llamada de dibujo)
    void setMat4(const char* name, const glm::mat4& m) const;
    void setFloat(const char* name, float v) const;

private:
    GLuint id_ = 0;
};

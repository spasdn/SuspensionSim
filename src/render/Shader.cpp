#include "render/Shader.h"

#include <glm/gtc/type_ptr.hpp>

static bool compileStage(GLenum type, const char* src, GLuint& out, std::string& error) {
    out = glCreateShader(type);
    glShaderSource(out, 1, &src, nullptr);
    glCompileShader(out);
    GLint ok = 0;
    glGetShaderiv(out, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(out, sizeof(log), nullptr, log);
        error = std::string(type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") + " shader: " + log;
        glDeleteShader(out);
        return false;
    }
    return true;
}

bool Shader::compile(const char* vs, const char* fs, std::string& error) {
    GLuint v = 0, f = 0;
    if (!compileStage(GL_VERTEX_SHADER, vs, v, error)) return false;
    if (!compileStage(GL_FRAGMENT_SHADER, fs, f, error)) { glDeleteShader(v); return false; }

    id_ = glCreateProgram();
    glAttachShader(id_, v);
    glAttachShader(id_, f);
    glLinkProgram(id_);
    glDeleteShader(v);  // ya están dentro del programa
    glDeleteShader(f);

    GLint ok = 0;
    glGetProgramiv(id_, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(id_, sizeof(log), nullptr, log);
        error = std::string("Link: ") + log;
        return false;
    }
    return true;
}

void Shader::destroy() {
    if (id_) glDeleteProgram(id_);
    id_ = 0;
}

void Shader::setMat4(const char* name, const glm::mat4& m) const {
    glUniformMatrix4fv(glGetUniformLocation(id_, name), 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::setFloat(const char* name, float v) const {
    glUniform1f(glGetUniformLocation(id_, name), v);
}

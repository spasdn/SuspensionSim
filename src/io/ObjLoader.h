#pragma once
// Lector mínimo de .obj (solo vértices "v" y caras "f"). Suficiente para dibujar
// un chasis o cualquier pieza exportada desde CAD (pasando por STL -> OBJ si hace falta).
#include <glm/glm.hpp>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace susp {

struct Mesh {
    std::vector<glm::vec3> vertices;
    std::vector<std::array<uint32_t, 3>> triangles;   // caras trianguladas
    std::vector<std::array<uint32_t, 2>> edges;       // aristas de los polígonos originales (sin diagonales)
    bool empty() const { return triangles.empty(); }
};

// 'scale' permite convertir unidades (p. ej. 1000 si el .obj está en metros)
bool loadObj(const std::string& path, Mesh& out, std::string& error, float scale = 1.0f);

} // namespace susp

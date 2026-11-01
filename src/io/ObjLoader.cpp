#include "io/ObjLoader.h"

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>

namespace susp {

bool loadObj(const std::string& path, Mesh& m, std::string& error, float scale) {
    std::ifstream f(path);
    if (!f) { error = "No se puede abrir " + path; return false; }
    m = Mesh{};
    std::set<std::pair<uint32_t, uint32_t>> edgeSet;
    std::string line;
    int lineNo = 0;
    while (std::getline(f, line)) {
        ++lineNo;
        std::istringstream ss(line);
        std::string tag;
        ss >> tag;
        if (tag == "v") {
            glm::vec3 p;
            ss >> p.x >> p.y >> p.z;
            m.vertices.push_back(p * scale);
        } else if (tag == "f") {
            // Formatos admitidos: "f 1 2 3", "f 1/1 2/2 3/3", "f 1/1/1 ...", "f 1//1 ..."
            std::vector<uint32_t> idx;
            std::string tok;
            while (ss >> tok) {
                long i = std::stol(tok.substr(0, tok.find('/')));
                if (i < 0) i = static_cast<long>(m.vertices.size()) + i + 1;  // índices negativos
                if (i < 1 || i > static_cast<long>(m.vertices.size())) {
                    error = "Índice fuera de rango en la línea " + std::to_string(lineNo);
                    return false;
                }
                idx.push_back(static_cast<uint32_t>(i - 1));
            }
            for (size_t k = 1; k + 1 < idx.size(); ++k)  // triangulación en abanico
                m.triangles.push_back({idx[0], idx[k], idx[k + 1]});
            for (size_t k = 0; k < idx.size(); ++k) {
                uint32_t a = idx[k], b = idx[(k + 1) % idx.size()];
                edgeSet.insert({std::min(a, b), std::max(a, b)});
            }
        }
    }
    for (auto& e : edgeSet) m.edges.push_back({e.first, e.second});
    if (m.triangles.empty()) { error = "El archivo no contiene caras: " + path; return false; }
    return true;
}

} // namespace susp

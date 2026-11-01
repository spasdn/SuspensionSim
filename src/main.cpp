// SuspensionSim — punto de entrada.
// Uso:  suspsim [ruta/al/modelo.json]
#include <cstdio>

#include "app/App.h"

int main(int argc, char** argv) {
    App app;
    if (!app.init(argc, argv)) {
        std::fprintf(stderr, "No se pudo iniciar la aplicacion (OpenGL 3.3 no disponible?)\n");
        return 1;
    }
    app.run();
    app.shutdown();
    return 0;
}

#pragma once
// =============================================================================
// App.h — La aplicación: ventana, bucle principal y estado del modelo
// -----------------------------------------------------------------------------
// Bucle principal (cada fotograma, ~60 veces por segundo):
//   1. Leer eventos (ratón, teclado)           -> glfwPollEvents()
//   2. Construir la interfaz ImGui             -> drawUi()   (Panels.cpp)
//   3. Mover la cámara con el ratón            -> handleCameraInput()
//   4. Borrar pantalla y dibujar la escena 3D  -> buildScene() + renderer.flush()
//   5. Dibujar la interfaz encima y presentar  -> ImGui render + glfwSwapBuffers()
// =============================================================================
#include <memory>
#include <string>
#include <vector>

#include "analysis/Analysis.h"
#include "analysis/Optimizer.h"
#include "io/ObjLoader.h"
#include "render/Camera.h"
#include "render/Renderer.h"
#include "render/SceneBuilder.h"

struct GLFWwindow;

class App {
public:
    bool init(int argc, char** argv);
    void run();
    void shutdown();

private:
    // ---- Modelo y cálculo (App.cpp) ----
    void loadModel(const std::string& path);
    void rebuildSolver();     // llamar cuando cambie cualquier punto o parámetro
    void updateKinematics();  // llamar cuando cambien travel / rack
    void runSweep();
    void generateReport();
    void log(const std::string& msg);

    // ---- Entrada ----
    void handleCameraInput();

    // ---- Interfaz (Panels.cpp) ----
    void drawUi();
    void panelModel();
    void panelBoundary();
    void panelResults();
    void panelReport();
    void panelOptimizer();
    void panelView();
    void windowPlots();

    // Ventana y dibujo
    GLFWwindow* window_ = nullptr;
    Renderer renderer_;
    Camera camera_;
    SceneOptions sceneOpt_;

    // Modelo
    bool modelLoaded_ = false;
    char modelPath_[512] = {};
    char reportDir_[512] = {};  // carpeta de informes (variable SUSPSIM_INFORMES o output/informes)
    susp::Suspension susp_;
    std::unique_ptr<susp::KinematicSolver> solver_;
    susp::KinematicInputs inputs_;
    susp::CornerState state_;
    susp::Metrics metrics_;
    std::vector<susp::Metrics> sweep_;
    susp::Mesh chassis_;
    bool animate_ = false;

    // Optimización
    std::vector<susp::DesignVariable> optVars_;
    susp::Objective optObjective_;
    susp::OptimizerSettings optSettings_;
    susp::OptimizerResult optResult_;
    bool hasOptResult_ = false;

    std::vector<std::string> log_;
};

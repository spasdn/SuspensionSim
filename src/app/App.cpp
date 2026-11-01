// IMPORTANTE: glad debe incluirse ANTES que GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "app/App.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "analysis/Report.h"
#include "io/SuspensionLoader.h"

namespace fs = std::filesystem;

// ============================================================================
// Inicialización
// ============================================================================
bool App::init(int argc, char** argv) {
    // 1) GLFW: ventana + contexto OpenGL 3.3 core
    if (!glfwInit()) return false;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_SAMPLES, 4);  // antialiasing (bordes más suaves)
    window_ = glfwCreateWindow(1600, 900, "SuspensionSim", nullptr, nullptr);
    if (!window_) { glfwTerminate(); return false; }
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);  // sincronizar con el monitor (60 fps)

    // 2) glad: cargar las funciones de OpenGL (sin esto, glXxx() no existen)
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) return false;
    glEnable(GL_MULTISAMPLE);

    // 3) Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = "suspsim_layout.ini";  // recuerda posición de las ventanas
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 4) Nuestro renderer
    std::string err;
    if (!renderer_.init(err)) { log(err); return false; }

    // 5) Modelo inicial
    std::string path = argc > 1 ? argv[1] : std::string(SUSPSIM_ROOT) + "/data/ejemplo_doble_trapecio.json";
    loadModel(path);

    // Carpeta de informes: la variable de entorno SUSPSIM_INFORMES manda (p. ej. la
    // carpeta Informes del proyecto en la bóveda de Obsidian); si no, output/informes.
    const char* envDir = std::getenv("SUSPSIM_INFORMES");
    const std::string dir = (envDir && *envDir) ? std::string(envDir)
                                                : (fs::path(SUSPSIM_ROOT) / "output" / "informes").string();
    std::snprintf(reportDir_, sizeof(reportDir_), "%s", dir.c_str());

    // Variable de optimización de ejemplo
    optVars_.push_back({"tie_rod_inner", 2, -30.0, 30.0});
    return true;
}

void App::shutdown() {
    renderer_.shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    if (window_) glfwDestroyWindow(window_);
    glfwTerminate();
}

// ============================================================================
// Bucle principal
// ============================================================================
void App::run() {
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (animate_ && modelLoaded_) {
            double mid = 0.5 * (susp_.bc.travelMax + susp_.bc.travelMin);
            double amp = 0.5 * (susp_.bc.travelMax - susp_.bc.travelMin);
            inputs_.travel = mid + amp * std::sin(glfwGetTime() * 1.5);
            updateKinematics();
        }

        drawUi();
        handleCameraInput();

        int w, h;
        glfwGetFramebufferSize(window_, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.11f, 0.12f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (h > 0) {
            renderer_.begin();
            if (modelLoaded_)
                buildScene(renderer_, susp_, state_, solver_->reference(), metrics_,
                           chassis_.empty() ? nullptr : &chassis_, sceneOpt_);
            float aspect = static_cast<float>(w) / static_cast<float>(h);
            renderer_.flush(camera_.projection(aspect) * camera_.view());
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window_);
    }
}

// ============================================================================
// Ratón: izquierdo = girar, derecho/central (o Mayús + izquierdo) = desplazar, rueda = zoom
// ============================================================================
void App::handleCameraInput() {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;  // el ratón está sobre un panel de ImGui
    const bool pan = io.MouseDown[1] || io.MouseDown[2] || (io.MouseDown[0] && io.KeyShift);
    if (pan)
        camera_.pan(io.MouseDelta.x, io.MouseDelta.y, io.DisplaySize.y);
    else if (io.MouseDown[0])
        camera_.orbit(io.MouseDelta.x, io.MouseDelta.y);
    if (io.MouseWheel != 0.0f) camera_.zoom(io.MouseWheel);
}

// ============================================================================
// Modelo y cálculo
// ============================================================================
void App::log(const std::string& msg) {
    log_.push_back(msg);
    if (log_.size() > 50) log_.erase(log_.begin());
}

void App::loadModel(const std::string& path) {
    std::snprintf(modelPath_, sizeof(modelPath_), "%s", path.c_str());
    susp::Suspension s;
    std::string err;
    if (!susp::loadSuspensionJson(path, s, err)) { log(err); return; }
    susp_ = s;
    inputs_ = {0.0, susp_.bc.rack};

    chassis_ = {};
    if (!susp_.chassisMeshPath.empty()) {
        if (susp::loadObj(susp_.chassisMeshPath, chassis_, err))
            log("Chasis cargado: " + std::to_string(chassis_.triangles.size()) + " triangulos");
        else
            log(err);
    }
    modelLoaded_ = true;
    hasOptResult_ = false;
    rebuildSolver();
    // Centrar la cámara en la rueda
    camera_.target = glm::vec3(0.0f, 0.0f, static_cast<float>(susp_.hp.wheelCenter.z));
    log("Modelo cargado: " + susp_.name);
}

void App::rebuildSolver() {
    solver_ = std::make_unique<susp::KinematicSolver>(susp_);
    updateKinematics();
    runSweep();
}

void App::updateKinematics() {
    if (!solver_) return;
    metrics_ = susp::analyze(*solver_, inputs_);
    state_ = solver_->solve(inputs_);
    if (!state_.valid) log("Sin solucion para recorrido " + std::to_string(inputs_.travel) + " mm");
}

void App::runSweep() {
    if (!solver_) return;
    sweep_ = susp::sweepTravel(*solver_, susp_.bc.travelMin, susp_.bc.travelMax, susp_.bc.steps, inputs_.rack);
    updateKinematics();  // restaurar el estado actual tras el barrido
}

void App::generateReport() {
    if (!solver_) return;
    susp::ReportInput in;
    in.susp = &susp_;
    in.staticMetrics = susp::analyze(*solver_, {0.0, 0.0});
    in.sweep = sweep_;
    if (hasOptResult_) in.notes = "Ultima optimizacion:\n\n```\n" + optResult_.message + "```\n";

    const std::string base = susp::defaultReportBaseName(susp_);
    const fs::path md  = fs::path(reportDir_) / (base + ".md");
    const fs::path csv = fs::path(reportDir_) / (base + ".csv");
    std::string err;
    if (susp::writeMarkdownReport(md.string(), in, err)) log("Informe: " + md.string());
    else log(err);
    if (susp::writeSweepCsv(csv.string(), sweep_, err)) log("CSV: " + csv.string());
    else log(err);
    updateKinematics();
}

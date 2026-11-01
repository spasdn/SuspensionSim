// =============================================================================
// Panels.cpp — Interfaz con Dear ImGui
// -----------------------------------------------------------------------------
// ImGui es "modo inmediato": cada fotograma se vuelve a declarar la interfaz.
//     if (ImGui::Button("Calcular")) { ... }   // el if se cumple el fotograma del clic
//     ImGui::SliderFloat("x", &x, 0, 1);       // modifica x directamente
// No hay que guardar punteros a botones ni registrar callbacks.
// Para ver TODO lo que ImGui sabe hacer: marca "Demo de ImGui" en el panel Vista.
// =============================================================================
#include "app/App.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <imgui.h>

#include "io/SuspensionLoader.h"

namespace {
bool showImGuiDemo = false;

// Fila de tabla "nombre | valor | unidad"
void row(const char* name, double v, const char* unit, int prec = 2) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(name);
    ImGui::TableSetColumnIndex(1);
    if (std::isinf(v)) ImGui::TextUnformatted("inf");
    else ImGui::Text("%.*f", prec, v);
    ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(unit);
}

bool dragVec3(const char* label, susp::Vec3& v) {
    return ImGui::DragScalarN(label, ImGuiDataType_Double, &v.x, 3, 0.5f, nullptr, nullptr, "%.1f");
}

const char* kAxisNames[] = {"X", "Y", "Z"};
const double kZero = 0.0, kOne = 1.0;
} // namespace

void App::drawUi() {
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(420, 860), ImGuiCond_FirstUseEver);
    ImGui::Begin("SuspensionSim");

    if (ImGui::CollapsingHeader("Modelo", ImGuiTreeNodeFlags_DefaultOpen))                 panelModel();
    if (ImGui::CollapsingHeader("Condiciones de contorno", ImGuiTreeNodeFlags_DefaultOpen)) panelBoundary();
    if (ImGui::CollapsingHeader("Resultados", ImGuiTreeNodeFlags_DefaultOpen))              panelResults();
    if (ImGui::CollapsingHeader("Informe"))                                                 panelReport();
    if (ImGui::CollapsingHeader("Optimizacion"))                                            panelOptimizer();
    if (ImGui::CollapsingHeader("Vista"))                                                   panelView();

    ImGui::Separator();
    ImGui::TextDisabled("Mensajes");
    ImGui::BeginChild("log", ImVec2(0, 90), ImGuiChildFlags_Borders);
    for (auto& l : log_) ImGui::TextWrapped("%s", l.c_str());
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
    ImGui::End();

    windowPlots();
    if (showImGuiDemo) ImGui::ShowDemoWindow(&showImGuiDemo);
}

// ----------------------------------------------------------------------------
void App::panelModel() {
    ImGui::InputText("Archivo", modelPath_, sizeof(modelPath_));
    if (ImGui::Button("Cargar")) loadModel(modelPath_);
    ImGui::SameLine();
    if (ImGui::Button("Guardar") && modelLoaded_) {
        std::string err;
        if (susp::saveSuspensionJson(modelPath_, susp_, err)) log(std::string("Guardado en ") + modelPath_);
        else log(err);
    }
    if (!modelLoaded_) return;

    ImGui::Text("%s  |  eje %s, lado %s", susp_.name.c_str(),
                susp_.axle == susp::Axle::Front ? "delantero" : "trasero",
                susp_.side == susp::Side::Left ? "izquierdo" : "derecho");

    if (ImGui::TreeNode("Puntos duros [mm]")) {
        ImGui::TextDisabled("Arrastra o haz doble clic para escribir");
        bool changed = false;
        for (const auto& name : susp::hardpointNames()) {
            if (!susp_.hp.hasDamper && name.rfind("damper", 0) == 0) continue;
            changed |= dragVec3(name.c_str(), *susp::hardpointByName(susp_.hp, name));
        }
        if (changed) rebuildSolver();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Rueda y vehiculo")) {
        bool c = false;
        c |= ImGui::InputDouble("Radio rueda [mm]", &susp_.tire.radius, 1, 10, "%.1f");
        c |= ImGui::InputDouble("Ancho rueda [mm]", &susp_.tire.width, 1, 10, "%.1f");
        c |= ImGui::InputDouble("Batalla [mm]", &susp_.vehicle.wheelbase, 10, 100, "%.0f");
        c |= ImGui::InputDouble("Altura CdG [mm]", &susp_.vehicle.cgHeight, 5, 50, "%.0f");
        c |= ImGui::SliderScalar("Frenada delante", ImGuiDataType_Double, &susp_.vehicle.frontBrakeBias,
                                 &kZero, &kOne, "%.2f");
        c |= ImGui::SliderScalar("Traccion delante", ImGuiDataType_Double, &susp_.vehicle.frontDriveFraction,
                                 &kZero, &kOne, "%.2f");
        if (c) rebuildSolver();
        ImGui::TreePop();
    }
}

// ----------------------------------------------------------------------------
void App::panelBoundary() {
    if (!modelLoaded_) return;
    auto& bc = susp_.bc;
    bool kin = false;
    kin |= ImGui::SliderScalar("Recorrido [mm]", ImGuiDataType_Double, &inputs_.travel, &bc.travelMin, &bc.travelMax, "%.1f");
    kin |= ImGui::SliderScalar("Cremallera [mm]", ImGuiDataType_Double, &inputs_.rack, &bc.rackMin, &bc.rackMax, "%.1f");
    if (ImGui::Button("Posicion de diseno")) { inputs_.travel = 0.0; inputs_.rack = 0.0; kin = true; }
    ImGui::SameLine();
    ImGui::Checkbox("Animar", &animate_);
    if (kin) updateKinematics();

    ImGui::SeparatorText("Barrido");
    bool sw = false;
    sw |= ImGui::InputDouble("Recorrido min", &bc.travelMin, 5, 10, "%.1f");
    sw |= ImGui::InputDouble("Recorrido max", &bc.travelMax, 5, 10, "%.1f");
    sw |= ImGui::SliderInt("Puntos", &bc.steps, 5, 101);
    if (sw || ImGui::Button("Recalcular barrido")) runSweep();
    ImGui::TextDisabled("Pendiente: balanceo, cabeceo, cargas -> ver roadmap");
}

// ----------------------------------------------------------------------------
void App::panelResults() {
    if (!modelLoaded_) return;
    const auto& m = metrics_;
    if (!m.valid) { ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "Sin solucion en esta posicion"); return; }
    if (ImGui::BeginTable("res", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn("Parametro");
        ImGui::TableSetupColumn("Valor", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 30);
        row("Caida", m.camber, "deg", 3);
        row("Convergencia (+in)", m.toe, "deg", 3);
        row("Avance", m.caster, "deg");
        row("KPI", m.kpi, "deg");
        row("Scrub radius", m.scrubRadius, "mm", 1);
        row("Avance mecanico", m.mechanicalTrail, "mm", 1);
        row("Brazo pivote en CR", m.kingpinOffsetWC, "mm", 1);
        row("Altura CB", m.rollCenterHeight, "mm", 1);
        row("FVSA", m.fvsaLength, "mm", 0);
        row("SVSA", m.svsaLength, "mm", 0);
        row("Anti frenada", m.antiBrake, "%", 1);
        row("Anti aceleracion", m.antiAccel, "%", 1);
        row("Semivia", m.halfTrack, "mm", 1);
        if (susp_.hp.hasDamper) {
            row("Long. amortiguador", m.damperLength, "mm", 1);
            row("Ratio instalacion", m.motionRatio, "-", 3);
        }
        ImGui::EndTable();
    }
    if (!sweep_.empty()) {
        ImGui::Text("Ganancia de caida: %.3f deg/10 mm", susp::gradientAtZero(sweep_, &susp::Metrics::camber) * 10);
        ImGui::Text("Bump steer: %.2f deg/m", susp::gradientAtZero(sweep_, &susp::Metrics::toe) * 1000);
    }
}

// ----------------------------------------------------------------------------
void App::panelReport() {
    ImGui::TextWrapped("Genera un informe Markdown (para Obsidian) y un CSV del barrido "
                       "en esta carpeta:");
    ImGui::InputText("Carpeta", reportDir_, sizeof(reportDir_));
    ImGui::TextDisabled("Fija la variable SUSPSIM_INFORMES para no escribirla cada vez");
    if (ImGui::Button("Generar informe") && modelLoaded_) generateReport();
}

// ----------------------------------------------------------------------------
void App::panelOptimizer() {
    if (!modelLoaded_) return;
    int type = static_cast<int>(optObjective_.type);
    const char* names[] = {susp::objectiveName(susp::ObjectiveType::MinBumpSteer),
                           susp::objectiveName(susp::ObjectiveType::TargetCamberGain),
                           susp::objectiveName(susp::ObjectiveType::TargetRollCenter),
                           susp::objectiveName(susp::ObjectiveType::MinRollCenterMigration)};
    if (ImGui::Combo("Objetivo", &type, names, IM_ARRAYSIZE(names)))
        optObjective_.type = static_cast<susp::ObjectiveType>(type);
    if (optObjective_.type == susp::ObjectiveType::TargetCamberGain ||
        optObjective_.type == susp::ObjectiveType::TargetRollCenter)
        ImGui::InputDouble("Valor objetivo", &optObjective_.target, 0.1, 1.0, "%.3f");

    ImGui::SeparatorText("Variables de diseno (desplazamiento permitido)");
    const auto& hpNames = susp::hardpointNames();
    for (size_t i = 0; i < optVars_.size(); ++i) {
        auto& v = optVars_[i];
        ImGui::PushID(static_cast<int>(i));
        ImGui::SetNextItemWidth(150);
        if (ImGui::BeginCombo("##hp", v.hardpoint.c_str())) {
            for (auto& n : hpNames)
                if (ImGui::Selectable(n.c_str(), n == v.hardpoint)) v.hardpoint = n;
            ImGui::EndCombo();
        }
        ImGui::SameLine(); ImGui::SetNextItemWidth(45);
        ImGui::Combo("##ax", &v.axis, kAxisNames, 3);
        ImGui::SameLine(); ImGui::SetNextItemWidth(110);
        ImGui::InputScalarN("##lim", ImGuiDataType_Double, &v.minOffset, 2, nullptr, nullptr, "%.0f");
        ImGui::SameLine();
        if (ImGui::SmallButton("x")) { optVars_.erase(optVars_.begin() + i); ImGui::PopID(); break; }
        ImGui::PopID();
    }
    if (ImGui::Button("+ Variable")) optVars_.push_back({"upper_ball_joint", 2, -15.0, 15.0});

    ImGui::SliderInt("Iteraciones max", &optSettings_.maxIterations, 20, 2000);
    if (ImGui::Button("Optimizar")) {
        optResult_ = susp::optimize(susp_, optVars_, optObjective_, optSettings_);
        hasOptResult_ = true;
        log(optResult_.ok ? "Optimizacion terminada" : "La optimizacion no mejoro el diseno");
    }
    if (hasOptResult_) {
        ImGui::TextWrapped("%s", optResult_.message.c_str());
        if (!optResult_.history.empty()) {
            std::vector<float> h(optResult_.history.begin(), optResult_.history.end());
            ImGui::PlotLines("Coste", h.data(), static_cast<int>(h.size()), 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(0, 60));
        }
        if (ImGui::Button("Aplicar resultado al modelo")) {
            susp_ = optResult_.best;
            rebuildSolver();
            log("Resultado aplicado. Usa 'Guardar' para escribirlo en el JSON.");
        }
    }
}

// ----------------------------------------------------------------------------
void App::panelView() {
    if (ImGui::Button("Frontal")) camera_.viewFront();
    ImGui::SameLine();
    if (ImGui::Button("Lateral")) camera_.viewSide();
    ImGui::SameLine();
    if (ImGui::Button("Planta")) camera_.viewTop();
    ImGui::SameLine();
    if (ImGui::Button("Iso")) camera_.viewIso();
    ImGui::Checkbox("Proyeccion ortogonal", &camera_.orthographic);
    ImGui::Checkbox("Rejilla", &sceneOpt_.showGrid);              ImGui::SameLine();
    ImGui::Checkbox("Ejes", &sceneOpt_.showAxes);                 ImGui::SameLine();
    ImGui::Checkbox("Puntos", &sceneOpt_.showHardpoints);
    ImGui::Checkbox("Lado opuesto", &sceneOpt_.showMirror);       ImGui::SameLine();
    ImGui::Checkbox("Posicion de diseno", &sceneOpt_.showReference);
    ImGui::Checkbox("Eje de pivote", &sceneOpt_.showKingpinAxis); ImGui::SameLine();
    ImGui::Checkbox("Chasis", &sceneOpt_.showChassis);
    ImGui::Checkbox("CI frontal / CB", &sceneOpt_.showFrontViewIC); ImGui::SameLine();
    ImGui::Checkbox("CI lateral", &sceneOpt_.showSideViewIC);
    ImGui::SliderFloat("Opacidad chasis", &sceneOpt_.chassisAlpha, 0.0f, 1.0f);
    ImGui::Checkbox("Demo de ImGui (para aprender)", &showImGuiDemo);
    ImGui::TextDisabled("Raton: izq = girar | der/central o Mayus+izq = mover | rueda = zoom");
}

// ----------------------------------------------------------------------------
void App::windowPlots() {
    if (sweep_.empty()) return;
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 430, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(420, 620), ImGuiCond_FirstUseEver);
    ImGui::Begin("Curvas frente a recorrido");
    ImGui::TextDisabled("Eje X: recorrido de %.0f a %.0f mm", susp_.bc.travelMin, susp_.bc.travelMax);

    auto plot = [&](const char* label, double susp::Metrics::*f, const char* unit) {
        std::vector<float> y;
        float lo = 1e30f, hi = -1e30f;
        for (auto& m : sweep_) {
            float v = m.valid ? static_cast<float>(m.*f) : 0.0f;
            y.push_back(v);
            lo = std::min(lo, v); hi = std::max(hi, v);
        }
        char overlay[96];
        std::snprintf(overlay, sizeof(overlay), "%.2f .. %.2f %s", lo, hi, unit);
        ImGui::PlotLines(label, y.data(), static_cast<int>(y.size()), 0, overlay, FLT_MAX, FLT_MAX, ImVec2(0, 80));
    };
    plot("Caida", &susp::Metrics::camber, "deg");
    plot("Convergencia", &susp::Metrics::toe, "deg");
    plot("Altura CB", &susp::Metrics::rollCenterHeight, "mm");
    plot("Semivia", &susp::Metrics::halfTrack, "mm");
    plot("Avance", &susp::Metrics::caster, "deg");
    if (susp_.hp.hasDamper) plot("Ratio inst.", &susp::Metrics::motionRatio, "-");
    ImGui::TextDisabled("Para graficas mejores: ImPlot (ver roadmap)");
    ImGui::End();
}

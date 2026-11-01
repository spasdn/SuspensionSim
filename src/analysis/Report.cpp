#include "analysis/Report.h"

#include <chrono>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace susp {
namespace {

std::string fmt(double v, int prec = 2) {
    if (std::isinf(v)) return "∞";
    if (std::isnan(v)) return "—";
    std::ostringstream o;
    o << std::fixed << std::setprecision(prec) << v;
    return o.str();
}

std::string nowString(const char* pattern) {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream o;
    o << std::put_time(&tm, pattern);
    return o.str();
}

std::string vec(const Vec3& v) {
    return "(" + fmt(v.x, 1) + ", " + fmt(v.y, 1) + ", " + fmt(v.z, 1) + ")";
}

double rangeOf(const std::vector<Metrics>& sw, double Metrics::*f) {
    double lo = 1e300, hi = -1e300;
    for (auto& m : sw) if (m.valid) { lo = std::min(lo, m.*f); hi = std::max(hi, m.*f); }
    return hi - lo;
}

} // namespace

std::string defaultReportBaseName(const Suspension& s) {
    return "Informe " + s.name + " " + nowString("%Y-%m-%d %H%M");
}

bool writeMarkdownReport(const std::string& path, const ReportInput& in, std::string& error) {
    if (!in.susp) { error = "Informe sin suspensión"; return false; }
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    std::ofstream f(path);
    if (!f) { error = "No se puede escribir " + path; return false; }
    const Suspension& s = *in.susp;
    const Metrics& m = in.staticMetrics;
    const auto& sw = in.sweep;

    // --- Cabecera YAML (propiedades de Obsidian) ---
    f << "---\n"
      << "tipo: informe\n"
      << "proyecto: \"[[SuspensionSim]]\"\n"
      << "modelo: \"" << s.name << "\"\n"
      << "fecha: " << nowString("%Y-%m-%d") << "\n"
      << "eje: " << (s.axle == Axle::Front ? "delantero" : "trasero") << "\n"
      << "lado: " << (s.side == Side::Left ? "izquierdo" : "derecho") << "\n"
      << "tags: [informe, suspension]\n"
      << "---\n\n";

    f << "# Informe de cinemática — " << s.name << "\n\n"
      << "Generado por SuspensionSim el " << nowString("%Y-%m-%d %H:%M") << ". "
      << "Criterios de signo en [[08 — Parámetros de suspensión]].\n\n";

    // --- Condiciones ---
    f << "## Condiciones de contorno\n\n"
      << "| Parámetro | Valor |\n|---|---|\n"
      << "| Recorrido analizado | " << fmt(s.bc.travelMin, 1) << " a " << fmt(s.bc.travelMax, 1) << " mm |\n"
      << "| Puntos del barrido | " << s.bc.steps << " |\n"
      << "| Cremallera | " << fmt(s.bc.rack, 1) << " mm |\n"
      << "| Radio de rueda | " << fmt(s.tire.radius, 1) << " mm |\n"
      << "| Batalla / altura CdG | " << fmt(s.vehicle.wheelbase, 0) << " / " << fmt(s.vehicle.cgHeight, 0) << " mm |\n"
      << "| Reparto de frenada delante | " << fmt(100 * s.vehicle.frontBrakeBias, 0) << " % |\n"
      << "| Tracción delantera | " << fmt(100 * s.vehicle.frontDriveFraction, 0) << " % |\n\n";

    // --- Estático ---
    f << "## Parámetros en posición de diseño\n\n"
      << "| Parámetro | Valor | Unidad |\n|---|---:|---|\n"
      << "| Caída | " << fmt(m.camber, 3) << " | ° |\n"
      << "| Convergencia (+ toe-in) | " << fmt(m.toe, 3) << " | ° |\n"
      << "| Avance (caster) | " << fmt(m.caster, 2) << " | ° |\n"
      << "| Inclinación del pivote (KPI) | " << fmt(m.kpi, 2) << " | ° |\n"
      << "| Radio de pivotamiento (scrub) | " << fmt(m.scrubRadius, 1) << " | mm |\n"
      << "| Avance mecánico (trail) | " << fmt(m.mechanicalTrail, 1) << " | mm |\n"
      << "| Brazo del pivote en centro de rueda | " << fmt(m.kingpinOffsetWC, 1) << " | mm |\n"
      << "| Altura del centro de balanceo | " << fmt(m.rollCenterHeight, 1) << " | mm |\n"
      << "| Brazo oscilante frontal (FVSA) | " << fmt(m.fvsaLength, 0) << " | mm |\n"
      << "| Brazo oscilante lateral (SVSA) | " << fmt(m.svsaLength, 0) << " | mm |\n"
      << "| Anti en frenada | " << fmt(m.antiBrake, 1) << " | % |\n"
      << "| Anti en aceleración | " << fmt(m.antiAccel, 1) << " | % |\n"
      << "| Semivía | " << fmt(m.halfTrack, 1) << " | mm |\n";
    if (s.hp.hasDamper)
        f << "| Longitud del amortiguador | " << fmt(m.damperLength, 1) << " | mm |\n"
          << "| Ratio de instalación | " << fmt(m.motionRatio, 3) << " | - |\n";
    f << "\nCentro instantáneo frontal: " << (m.fvicAtInfinity ? "en el infinito (brazos paralelos)" : vec(m.fvic))
      << "  \nCentro instantáneo lateral: " << (m.svicAtInfinity ? "en el infinito" : vec(m.svic)) << "\n\n";

    // --- Ganancias ---
    if (!sw.empty()) {
        f << "## Ganancias en torno a la posición de diseño\n\n"
          << "| Magnitud | Valor | Unidad |\n|---|---:|---|\n"
          << "| Ganancia de caída | " << fmt(gradientAtZero(sw, &Metrics::camber) * 25.4, 3) << " | °/pulgada |\n"
          << "| Ganancia de caída | " << fmt(gradientAtZero(sw, &Metrics::camber) * 10, 3) << " | °/10 mm |\n"
          << "| Bump steer | " << fmt(gradientAtZero(sw, &Metrics::toe) * 1000, 3) << " | °/m |\n"
          << "| Migración vertical del CB | " << fmt(gradientAtZero(sw, &Metrics::rollCenterHeight), 3) << " | mm/mm |\n"
          << "| Variación de semivía | " << fmt(gradientAtZero(sw, &Metrics::halfTrack) * 10, 3) << " | mm/10 mm |\n"
          << "| Variación de caída en el barrido | " << fmt(rangeOf(sw, &Metrics::camber), 3) << " | ° |\n"
          << "| Variación de convergencia en el barrido | " << fmt(rangeOf(sw, &Metrics::toe), 3) << " | ° |\n\n";

        // --- Tabla del barrido ---
        f << "## Barrido de recorrido\n\n"
          << "| Recorrido [mm] | Caída [°] | Conv. [°] | Avance [°] | KPI [°] | Scrub [mm] | h CB [mm] | Semivía [mm] | Anti fren. [%] |";
        if (s.hp.hasDamper) f << " MR [-] |";
        f << "\n|---:|---:|---:|---:|---:|---:|---:|---:|---:|";
        if (s.hp.hasDamper) f << "---:|";
        f << "\n";
        for (auto& r : sw) {
            if (!r.valid) { f << "| " << fmt(r.travel, 1) << " | sin solución |||||||| \n"; continue; }
            f << "| " << fmt(r.travel, 1) << " | " << fmt(r.camber, 3) << " | " << fmt(r.toe, 3)
              << " | " << fmt(r.caster, 2) << " | " << fmt(r.kpi, 2) << " | " << fmt(r.scrubRadius, 1)
              << " | " << fmt(r.rollCenterHeight, 1) << " | " << fmt(r.halfTrack, 1)
              << " | " << fmt(r.antiBrake, 1) << " |";
            if (s.hp.hasDamper) f << " " << fmt(r.motionRatio, 3) << " |";
            f << "\n";
        }
        f << "\n";
    }

    // --- Puntos duros ---
    f << "## Puntos duros (mm, ISO 8855)\n\n| Punto | X | Y | Z |\n|---|---:|---:|---:|\n";
    Hardpoints hp = s.hp;
    for (const auto& n : hardpointNames()) {
        if (!hp.hasDamper && n.rfind("damper", 0) == 0) continue;
        Vec3* p = hardpointByName(hp, n);
        f << "| " << n << " | " << fmt(p->x, 1) << " | " << fmt(p->y, 1) << " | " << fmt(p->z, 1) << " |\n";
    }

    if (!in.notes.empty()) f << "\n## Notas\n\n" << in.notes << "\n";
    return true;
}

bool writeSweepCsv(const std::string& path, const std::vector<Metrics>& sw, std::string& error) {
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    std::ofstream f(path);
    if (!f) { error = "No se puede escribir " + path; return false; }
    // Separador ';' y punto decimal: se abre bien en LibreOffice/Excel con configuración española
    f << "travel_mm;camber_deg;toe_deg;caster_deg;kpi_deg;scrub_mm;trail_mm;rc_height_mm;"
         "fvsa_mm;anti_brake_pct;anti_accel_pct;half_track_mm;wheel_center_x_mm;damper_len_mm;motion_ratio\n";
    for (auto& m : sw) {
        if (!m.valid) continue;
        f << m.travel << ";" << m.camber << ";" << m.toe << ";" << m.caster << ";" << m.kpi << ";"
          << m.scrubRadius << ";" << m.mechanicalTrail << ";" << m.rollCenterHeight << ";"
          << m.fvsaLength << ";" << m.antiBrake << ";" << m.antiAccel << ";" << m.halfTrack << ";"
          << m.wheelCenterX << ";" << m.damperLength << ";" << m.motionRatio << "\n";
    }
    return true;
}

} // namespace susp

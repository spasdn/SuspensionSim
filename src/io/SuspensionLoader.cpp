#include "io/SuspensionLoader.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace susp {
using json = nlohmann::json;
namespace fs = std::filesystem;

namespace {
Vec3 readVec(const json& j, const char* key) {
    if (!j.contains(key)) throw std::runtime_error(std::string("falta el punto '") + key + "'");
    const json& a = j.at(key);
    if (!a.is_array() || a.size() != 3)
        throw std::runtime_error(std::string("'") + key + "' debe ser [x, y, z]");
    return Vec3(a[0].get<double>(), a[1].get<double>(), a[2].get<double>());
}
json vecToJson(const Vec3& v) { return json::array({v.x, v.y, v.z}); }
} // namespace

bool loadSuspensionJson(const std::string& path, Suspension& s, std::string& error) {
    try {
        std::ifstream f(path);
        if (!f) { error = "No se puede abrir " + path; return false; }
        json j = json::parse(f, nullptr, true, /*ignore_comments=*/true);

        s = Suspension{};
        s.name = j.value("name", fs::path(path).stem().string());
        s.axle = j.value("axle", "front") == "rear" ? Axle::Rear : Axle::Front;
        s.side = j.value("side", "left") == "right" ? Side::Right : Side::Left;

        const json& hp = j.at("hardpoints");
        s.hp.lcaFront       = readVec(hp, "lca_front");
        s.hp.lcaRear        = readVec(hp, "lca_rear");
        s.hp.ucaFront       = readVec(hp, "uca_front");
        s.hp.ucaRear        = readVec(hp, "uca_rear");
        s.hp.tieRodInner    = readVec(hp, "tie_rod_inner");
        s.hp.lowerBallJoint = readVec(hp, "lower_ball_joint");
        s.hp.upperBallJoint = readVec(hp, "upper_ball_joint");
        s.hp.tieRodOuter    = readVec(hp, "tie_rod_outer");
        s.hp.wheelCenter    = readVec(hp, "wheel_center");
        s.hp.spindlePoint   = readVec(hp, "spindle_point");

        if (j.contains("damper")) {
            const json& d = j["damper"];
            s.hp.hasDamper = true;
            s.hp.damperChassis = readVec(d, "chassis");
            s.hp.damperArm     = readVec(d, "arm");
            std::string at = d.value("attached_to", "lower_arm");
            s.hp.damperAttach = at == "upper_arm" ? Attach::UpperArm
                              : at == "upright"   ? Attach::Upright : Attach::LowerArm;
        }
        if (j.contains("tire")) {
            s.tire.radius = j["tire"].value("radius", s.tire.radius);
            s.tire.width  = j["tire"].value("width",  s.tire.width);
        }
        if (j.contains("vehicle")) {
            const json& v = j["vehicle"];
            s.vehicle.wheelbase          = v.value("wheelbase", s.vehicle.wheelbase);
            s.vehicle.cgHeight           = v.value("cg_height", s.vehicle.cgHeight);
            s.vehicle.mass               = v.value("mass", s.vehicle.mass);
            s.vehicle.frontBrakeBias     = v.value("front_brake_bias", s.vehicle.frontBrakeBias);
            s.vehicle.frontDriveFraction = v.value("front_drive_fraction", s.vehicle.frontDriveFraction);
        }
        if (j.contains("boundary")) {
            const json& b = j["boundary"];
            s.bc.travelMin = b.value("travel_min", s.bc.travelMin);
            s.bc.travelMax = b.value("travel_max", s.bc.travelMax);
            s.bc.steps     = b.value("steps", s.bc.steps);
            s.bc.rack      = b.value("rack", s.bc.rack);
            s.bc.rackMin   = b.value("rack_min", s.bc.rackMin);
            s.bc.rackMax   = b.value("rack_max", s.bc.rackMax);
        }
        if (j.contains("chassis_mesh")) {
            // Ruta relativa al propio archivo JSON
            fs::path mesh = fs::path(path).parent_path() / j["chassis_mesh"].get<std::string>();
            s.chassisMeshPath = mesh.lexically_normal().string();
        }
        return true;
    } catch (const std::exception& e) {
        error = std::string("Error leyendo ") + path + ": " + e.what();
        return false;
    }
}

bool saveSuspensionJson(const std::string& path, const Suspension& s, std::string& error) {
    try {
        json j;
        j["name"] = s.name;
        j["units"] = "mm";
        j["axle"] = s.axle == Axle::Front ? "front" : "rear";
        j["side"] = s.side == Side::Left ? "left" : "right";
        json hp;
        hp["lca_front"] = vecToJson(s.hp.lcaFront);
        hp["lca_rear"] = vecToJson(s.hp.lcaRear);
        hp["uca_front"] = vecToJson(s.hp.ucaFront);
        hp["uca_rear"] = vecToJson(s.hp.ucaRear);
        hp["tie_rod_inner"] = vecToJson(s.hp.tieRodInner);
        hp["lower_ball_joint"] = vecToJson(s.hp.lowerBallJoint);
        hp["upper_ball_joint"] = vecToJson(s.hp.upperBallJoint);
        hp["tie_rod_outer"] = vecToJson(s.hp.tieRodOuter);
        hp["wheel_center"] = vecToJson(s.hp.wheelCenter);
        hp["spindle_point"] = vecToJson(s.hp.spindlePoint);
        j["hardpoints"] = hp;
        if (s.hp.hasDamper) {
            const char* at = s.hp.damperAttach == Attach::UpperArm ? "upper_arm"
                           : s.hp.damperAttach == Attach::Upright  ? "upright" : "lower_arm";
            j["damper"] = {{"chassis", vecToJson(s.hp.damperChassis)},
                           {"arm", vecToJson(s.hp.damperArm)},
                           {"attached_to", at}};
        }
        j["tire"] = {{"radius", s.tire.radius}, {"width", s.tire.width}};
        j["vehicle"] = {{"wheelbase", s.vehicle.wheelbase}, {"cg_height", s.vehicle.cgHeight},
                        {"mass", s.vehicle.mass}, {"front_brake_bias", s.vehicle.frontBrakeBias},
                        {"front_drive_fraction", s.vehicle.frontDriveFraction}};
        j["boundary"] = {{"travel_min", s.bc.travelMin}, {"travel_max", s.bc.travelMax},
                         {"steps", s.bc.steps}, {"rack", s.bc.rack},
                         {"rack_min", s.bc.rackMin}, {"rack_max", s.bc.rackMax}};
        if (!s.chassisMeshPath.empty()) {
            fs::path rel = fs::path(s.chassisMeshPath).lexically_relative(fs::path(path).parent_path());
            j["chassis_mesh"] = rel.generic_string();
        }
        std::ofstream f(path);
        if (!f) { error = "No se puede escribir " + path; return false; }
        f << j.dump(2) << "\n";
        return true;
    } catch (const std::exception& e) {
        error = e.what();
        return false;
    }
}

} // namespace susp

#include "core/Suspension.h"

namespace susp {

const std::vector<std::string>& hardpointNames() {
    static const std::vector<std::string> names = {
        "lca_front", "lca_rear", "uca_front", "uca_rear", "tie_rod_inner",
        "lower_ball_joint", "upper_ball_joint", "tie_rod_outer",
        "wheel_center", "spindle_point", "damper_chassis", "damper_arm"};
    return names;
}

Vec3* hardpointByName(Hardpoints& hp, const std::string& n) {
    if (n == "lca_front")        return &hp.lcaFront;
    if (n == "lca_rear")         return &hp.lcaRear;
    if (n == "uca_front")        return &hp.ucaFront;
    if (n == "uca_rear")         return &hp.ucaRear;
    if (n == "tie_rod_inner")    return &hp.tieRodInner;
    if (n == "lower_ball_joint") return &hp.lowerBallJoint;
    if (n == "upper_ball_joint") return &hp.upperBallJoint;
    if (n == "tie_rod_outer")    return &hp.tieRodOuter;
    if (n == "wheel_center")     return &hp.wheelCenter;
    if (n == "spindle_point")    return &hp.spindlePoint;
    if (n == "damper_chassis")   return &hp.damperChassis;
    if (n == "damper_arm")       return &hp.damperArm;
    return nullptr;
}

} // namespace susp

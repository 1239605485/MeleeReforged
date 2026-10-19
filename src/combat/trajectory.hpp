#pragma once
#include "attack_types.hpp"
namespace mr {
AttackPose evaluatePose(const WeaponProfile &, int combo, float progress,
                        Vec2 grip, int direction, float gravity,
                        float bladeLength, bool charged = false);
BladeSegment sampleBlade(const AttackPose &, float length);
} // namespace mr

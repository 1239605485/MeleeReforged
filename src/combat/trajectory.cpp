#include "trajectory.hpp"
#include <algorithm>
namespace mr {
AttackPose evaluatePose(const WeaponProfile &weapon, int combo, float u,
                        Vec2 grip, int direction, float gravity,
                        float bladeLength) {
  AttackPose p{};
  p.active = true;
  p.combo = combo;
  p.progress = std::clamp(u, 0.f, 1.f);
  p.grip = grip;
  p.direction = direction;
  p.gravity = gravity;
  p.bladeLength = bladeLength;
  const auto &s = weapon.swings[combo];
  float t{};
  if (u < s.windup) {
    p.phase = Phase::Windup;
    t = 0;
  } else if (u < s.windup + s.swing) {
    p.phase = Phase::Swing;
    p.damaging = true;
    t = (u - s.windup) / s.swing;
  } else {
    p.phase = Phase::Recovery;
    t = 1;
  }
  t = t * t * (3 - 2 * t);
  p.angle = s.startAngle + (s.endAngle - s.startAngle) * t;
  p.tip = grip + Vec2{std::cos(p.angle) * bladeLength * direction,
                      std::sin(p.angle) * bladeLength * gravity};
  return p;
}
BladeSegment sampleBlade(const AttackPose &p, float n) {
  return {p.grip, p.grip + Vec2{std::cos(p.angle) * n * p.direction,
                                std::sin(p.angle) * n * p.gravity}};
}
} // namespace mr

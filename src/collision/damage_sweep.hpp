#pragma once
#include "combat/trajectory.hpp"
#include "weapons/weapon_registry.hpp"
#include <algorithm>
namespace mr {
struct DamageSweep {
  std::array<BladeSegment, 33> segments{};
  std::size_t count{};
  std::uint64_t attackId{};
};
inline DamageSweep collectDamageSweep(const AttackPose &previous,
                                      const AttackPose &current) {
  DamageSweep out;
  if (!current.active || current.charging)
    return out;
  const auto *weapon = findWeapon(current.itemId);
  if(!weapon)return out;
  AttackPose chosen = current;
  float start = previous.active && !previous.charging && previous.attackId == current.attackId
                    ? previous.progress
                    : 0.f;
  float end = current.progress;
  // Preserve a swing crossed just before a cycle boundary; never turn skipped
  // cycles into burst damage.
  if (previous.active && !previous.charging && previous.attackId != current.attackId) {
    auto &old = previous.charged ? weapon->heavy[previous.combo] : weapon->swings[previous.combo];
    if (previous.progress < old.windup + old.swing) {
      chosen = previous;
      start = previous.progress;
      end = 1;
    }
  }
  const auto &profile = chosen.charged ? weapon->heavy[chosen.combo] : weapon->swings[chosen.combo];
  float lo = std::max(start, profile.windup),
        hi = std::min(end, profile.windup + profile.swing);
  if (hi <= lo || end <= start)
    return out;
  out.attackId = chosen.attackId;
  int steps = std::clamp(int(std::ceil((hi - lo) / profile.swing * 32)), 1, 32);
  for (int i = 0; i <= steps; ++i) {
    float u = lo + (hi - lo) * float(i) / float(steps);
    float mix = (u - start) / (end - start);
    Vec2 grip = previous.active && !previous.charging
                    ? (previous.grip-previous.handOffset) +
                      ((current.grip-current.handOffset)-(previous.grip-previous.handOffset))*mix
                    : current.grip-current.handOffset;
    auto p = evaluatePose(*weapon, chosen.combo, u, grip, chosen.direction,
                          chosen.gravity, chosen.bladeLength, chosen.charged);
    out.segments[out.count++] = sampleBlade(p, chosen.bladeLength);
  }
  return out;
}
} // namespace mr

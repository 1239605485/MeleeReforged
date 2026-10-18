#pragma once
#include "combat/attack_types.hpp"
#include <cstddef>
namespace mr {
struct TrailPoint {
  AttackPose pose;
  float time{};
};
class TrailBuffer {
  std::array<TrailPoint, 28> points_{};
  std::size_t count_{};
  std::uint64_t tick_{~std::uint64_t(0)};
  float time_{};

public:
  void push(std::uint64_t, const AttackPose &);
  void clear();
  std::size_t size() const { return count_; }
  const TrailPoint &operator[](std::size_t i) const { return points_[i]; }
  float time() const { return time_; }
};
class EffectEvents {
  std::array<std::uint64_t, 64> keys_{};
  std::size_t count_{}, next_{};

public:
  bool emitOnce(std::uint64_t k);
  void clear() {
    count_ = next_ = 0;
    keys_ = {};
  }
};
} // namespace mr

#pragma once
#include "combat/attack_types.hpp"
namespace mr {
bool sweepIntersects(const BladeSegment &, const BladeSegment &, const Rect &);
class DamageWindow {
  struct Entry {
    std::uint64_t attack{}, generation{};
    bool set{};
  };
  std::array<Entry, 200> hits_{};

public:
  bool shouldProcess(std::uint64_t, int, std::uint64_t) const;
  void markProcessed(std::uint64_t, int, std::uint64_t);
  void clear() { hits_ = {}; }
};
} // namespace mr

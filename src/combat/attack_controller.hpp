#pragma once
#include "attack_types.hpp"
#include <limits>
namespace mr {
class AttackController {
  AttackPose pose_{};
  float elapsed_{}, idle_{};
  int nextCombo_{}, lockedDirection_{1};
  float lockedGravity_{1};
  std::uint64_t lastTick_{std::numeric_limits<std::uint64_t>::max()}, world_{},
      player_{}, serial_{};

public:
  AttackPose advance(const AttackInput &);
  void reset();
  const AttackPose &pose() const { return pose_; }
};
} // namespace mr

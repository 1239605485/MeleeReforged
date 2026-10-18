#include "attack_controller.hpp"
#include "trajectory.hpp"
#include "weapons/weapon_registry.hpp"
namespace mr {
void AttackController::reset() {
  pose_ = {};
  elapsed_ = idle_ = 0;
  nextCombo_ = 0;
  lastTick_ = std::numeric_limits<std::uint64_t>::max();
}
AttackPose AttackController::advance(const AttackInput &in) {
  if (world_ != in.worldGeneration || player_ != in.playerGeneration) {
    reset();
    world_ = in.worldGeneration;
    player_ = in.playerGeneration;
  }
  auto w = findWeapon(in.itemId);
  if (!w || !in.alive || !in.inWorld || !in.singlePlayer ||
      !std::isfinite(in.dtSeconds) || in.dtSeconds < 0 ||
      !std::isfinite(in.cycleSeconds) || in.cycleSeconds <= 0 ||
      !std::isfinite(in.bladeLength) || in.bladeLength <= 0 ||
      !std::isfinite(in.grip.x) || !std::isfinite(in.grip.y)) {
    reset();
    return pose_;
  }
  if (lastTick_ == in.tick)
    return pose_;
  lastTick_ = in.tick;
  if (!pose_.active) {
    idle_ += in.dtSeconds;
    if (idle_ > .6f)
      nextCombo_ = 0;
    if (!in.attacking)
      return pose_;
    elapsed_ = 0;
    pose_.active = true;
    pose_.combo = nextCombo_;
    pose_.attackId = ++serial_;
    lockedDirection_ = in.direction < 0 ? -1 : 1;
    lockedGravity_ = in.gravity < 0 ? -1.f : 1.f;
  } else
    elapsed_ += in.dtSeconds;
  // Gameplay adapters should feed fixed simulation ticks. Carry at most three
  // cycles; no unbounded catch-up.
  for (int n = 0; elapsed_ + 1e-6f >= in.cycleSeconds && n < 3; ++n) {
    elapsed_ -= in.cycleSeconds;
    nextCombo_ = (pose_.combo + 1) % 3;
    idle_ = 0;
    if (!in.attacking) {
      pose_ = {};
      return pose_;
    }
    pose_.combo = nextCombo_;
    pose_.attackId = ++serial_;
    lockedDirection_ = in.direction < 0 ? -1 : 1;
    lockedGravity_ = in.gravity < 0 ? -1.f : 1.f;
  }
  if (elapsed_ >= in.cycleSeconds)
    elapsed_ = std::fmod(elapsed_, in.cycleSeconds);
  auto id = pose_.attackId;
  auto combo = pose_.combo;
  pose_ = evaluatePose(*w, combo, elapsed_ / in.cycleSeconds, in.grip,
                       lockedDirection_, lockedGravity_, in.bladeLength);
  pose_.attackId = id;
  pose_.tick = in.tick;
  pose_.ageSeconds = elapsed_;
  return pose_;
}
} // namespace mr

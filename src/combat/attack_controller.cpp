#include <algorithm>
#include "attack_controller.hpp"
#include "trajectory.hpp"
#include "weapons/weapon_registry.hpp"
namespace mr {
void AttackController::reset() {
  pose_ = {};
  charged_ = false;
  elapsed_ = idle_ = 0;
  hitStopRemaining_=0;stoppedAttack_=0;
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
  if (in.charging && (!pose_.active || pose_.charging)) {
    float level=std::clamp(in.chargeLevel,0.f,1.f);
    pose_=evaluatePose(*w,0,w->heavy[0].windup*level,in.grip,in.direction,in.gravity,in.bladeLength,true);
    pose_.phase=Phase::Charging;pose_.charging=true;pose_.damaging=false;
    pose_.tick=in.tick;
    pose_.chargeLevel=std::clamp(in.chargeLevel,0.f,1.f);
    pose_.tip=pose_.grip+Vec2{std::cos(pose_.angle)*in.bladeLength*in.direction,
                            std::sin(pose_.angle)*in.bladeLength*in.gravity};
    return pose_;
  }
  if (pose_.charging) pose_={};
  if (!pose_.active) {
    idle_ += in.dtSeconds;
    if (idle_ > .6f)
      nextCombo_ = 0;
    if (!in.attacking)
      return pose_;
    elapsed_ = 0;
    pose_.active = true;
    charged_ = in.chargedAttack;
    pose_.combo = charged_ ? 0 : nextCombo_;
    pose_.attackId = ++serial_;
    lockedDirection_ = in.direction < 0 ? -1 : 1;
    lockedGravity_ = in.gravity < 0 ? -1.f : 1.f;
  } else {
    if(in.hitConfirmed && pose_.damaging && stoppedAttack_!=pose_.attackId) {
      stoppedAttack_=pose_.attackId;
      hitStopRemaining_=charged_?.05f:(2.f/60.f);
    }
    float paused=std::min(hitStopRemaining_,in.dtSeconds);
    hitStopRemaining_-=paused;
    elapsed_+=in.dtSeconds-paused;
  }
  // Gameplay adapters should feed fixed simulation ticks. Carry at most three
  // cycles; no unbounded catch-up.
  float cycle = in.cycleSeconds * (charged_ ? 1.5f : 1.f);
  for (int n = 0; elapsed_ + 1e-6f >= cycle && n < 3; ++n) {
    elapsed_ -= cycle;
    nextCombo_ = (pose_.combo + 1) % 3;
    if(in.attacking && in.chargedAttack && !charged_) {
      charged_=true;
      nextCombo_=0;
      pose_.combo=0;
      pose_.attackId=++serial_;
      elapsed_=0;
      cycle=in.cycleSeconds*1.5f;
      lockedDirection_=in.direction<0?-1:1;
      lockedGravity_=in.gravity<0?-1.f:1.f;
      break;
    }
    idle_ = 0;
    if (!in.attacking || (charged_ && nextCombo_==0)) {
      pose_ = {};
      return pose_;
    }
    pose_.combo = nextCombo_;
    pose_.attackId = ++serial_;
    lockedDirection_ = in.direction < 0 ? -1 : 1;
    lockedGravity_ = in.gravity < 0 ? -1.f : 1.f;
  }
  if (elapsed_ >= cycle)
    elapsed_ = std::fmod(elapsed_, cycle);
  auto id = pose_.attackId;
  auto combo = pose_.combo;
  pose_ = evaluatePose(*w, combo, elapsed_ / cycle, in.grip,
                       lockedDirection_, lockedGravity_, in.bladeLength, charged_);
  pose_.attackId = id;
  pose_.tick = in.tick;
  pose_.ageSeconds = elapsed_;
  return pose_;
}
} // namespace mr

#pragma once
#include "attack_controller.hpp"
#include "attack_button.hpp"
#include "carry_state.hpp"
#include "sword_wave.hpp"
#include "carry/state.hpp"
#include "collision/blade_sweep.hpp"
#include "render/trail_buffer.hpp"
namespace mr {
struct Impact {
  Vec2 position;
  std::uint64_t tick{};
  bool active{};
};
struct PlayerState {
  AttackController controller;
  AttackButton button;
  CarryState carry;
  CarryPresentationState presentation;
  std::array<SwordWave,4> waves{};
  std::uint64_t launchedAttack{},lastWaveTick{~std::uint64_t(0)};
  bool drawingFromWaist{};
  bool pendingNormal{}, pendingHeavy{}, bodyApplied{};
  bool hitConfirmed{};
  Vec2 baseBodyPosition{};
  Vec2 baseLegPosition{};
  float baseLegRotation{};
  // Exact byte layout of Player.CompositeArmData, resolved by field size.
  std::array<unsigned char,12> baseFrontArm{},baseBackArm{};
  float baseBodyRotation{};
  float cycleSeconds{.3f};
  AttackPose previous{}, current{};
  TrailBuffer trail;
  DamageWindow damage;
  std::uint64_t lastTick{}, generation{};
  std::uint64_t steppedAttack{};
  std::uintptr_t identity{}, world{};
  bool seen{};
  int combatItemId{};
  std::array<Impact, 8> impacts{};
  std::size_t nextImpact{};
  std::uintptr_t pendingNpc{};
  int pendingLife{}, pendingSlot{-1};
  std::uint64_t pendingAttack{};
  void clear() { *this = PlayerState{}; }
  void clearEquipment(){
    auto who=identity,where=world;auto gen=generation;
    clear();identity=who;world=where;generation=gen;
  }
};
class PlayerStates {
  std::array<PlayerState, 256> states_{};

public:
  PlayerState *get(int slot) {
    return slot >= 0 && slot < 256 ? &states_[slot] : nullptr;
  }
  void clear() {
    for (auto &s : states_)
      s.clear();
  }
};
} // namespace mr

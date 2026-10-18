#pragma once
#include "attack_controller.hpp"
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
  AttackPose previous{}, current{};
  TrailBuffer trail;
  DamageWindow damage;
  std::uint64_t lastTick{}, generation{};
  std::uintptr_t identity{}, world{};
  bool seen{};
  std::array<Impact, 8> impacts{};
  std::size_t nextImpact{};
  std::uintptr_t pendingNpc{};
  int pendingLife{}, pendingSlot{-1};
  std::uint64_t pendingAttack{};
  void clear() { *this = PlayerState{}; }
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

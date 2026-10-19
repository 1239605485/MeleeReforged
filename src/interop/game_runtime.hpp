#pragma once
#include "combat/player_states.hpp"
#include "hook_set.hpp"
#include "runtime_api.hpp"
#include <atomic>
#include "weapon_action.h"
namespace mr {
class GameRuntime {
  RuntimeApi api_;
  kernel_mod_handle_t *weaponApiOwner_{};
  bool weaponApiAttached_{};
  PlayerStates states_;
  std::atomic<bool> ready_{false};
  bool logged_{};
  bool wasInWorld_{}, renderFailed_{};
  std::array<std::uint64_t, 200> npcGenerations_{};

public:
  bool start();
  void startWeaponApi(kernel_mod_handle_t*);
  void stopWeaponApi();
  bool updateWeaponApi(Handle,PlayerState&,Handle);
  void cancelWeaponApiPlayer(Handle);
  void restoreWeaponApiPoses();
  void drawWeaponApi(Handle,uint32_t);
  bool drawWeaponApiBase(Handle,Handle,PlayerState&);
  void stop();
  bool validScene();
  PlayerState *state(Handle);
  RuntimeApi &api() { return api_; }
  void update(Handle);
  void action(Handle);
  bool hitbox(Handle, void **);
  bool hitNpc(Handle, void **);
  void afterHit(Handle, void **);
  bool draw(Handle);
  void drawBack(Handle);
  void updateWaves(Handle,PlayerState &);
  void launchWave(Handle,PlayerState &,Handle);
  void drawWaves(Handle,const PlayerState &,Vec2,Handle);
  void drawCarry(Handle,Handle,PlayerState &,bool);
  void updateGenericCarry(Handle,PlayerState &,Handle);
  bool drawGenericCarry(Handle,Handle,PlayerState &,bool);
  void impact(PlayerState &, Vec2);
  bool resourcesReady();
  void apply(Handle, PlayerState &);
  void restorePose(Handle);
  void frame();
  void resetNpc(int);
  void renderFailure() { renderFailed_ = true; }
};
GameRuntime &runtime();
void log(const char *message);
} // namespace mr

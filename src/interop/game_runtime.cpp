#include "game_runtime.hpp"
#include "native_hooks.hpp"
namespace mr {
GameRuntime &runtime() {
  static GameRuntime r;
  return r;
}
bool GameRuntime::start() {
  if (ready_.load())
    return true;
  auto report = api_.resolve();
  if (!report.compatible) {
    log(report.detail);
    log("MeleeReforged disabled: incompatible runtime; vanilla remains "
        "active.");
    return false;
  }
  if (!installNativeHooks(api_)) {
    log("Native hook installation failed; initialization stopped.");
    return false;
  }
  ready_.store(true);
  log("MeleeReforged 0.5.0 active: single-player weapon/tool carry and wooden sword; A three moves, C charge/release + manual followups; waist carry, heavy damage multipliers, gold sword wave, independent arms and textured slash.");
  return true;
}
void GameRuntime::stop() {
  ready_.store(false);
  removeNativeHooks();
  states_.clear();
}
} // namespace mr

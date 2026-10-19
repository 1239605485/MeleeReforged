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
  log("MeleeReforged 0.7.0 active: single-player; katana single draw cut on tap release, .55s charge on full hold release; calibrated mirrored grip, textured fan and scabbard; wooden sword preserved.");
  return true;
}
void GameRuntime::stop() {
  ready_.store(false);
  removeNativeHooks();
  states_.clear();
}
} // namespace mr

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
  log("MeleeReforged 0.6.1 active: single-player; katana attack hold repeats combo; down+attack press selects charge, full release performs one iai cut; swept textured trail. Wooden sword and equipment carry preserved.");
  return true;
}
void GameRuntime::stop() {
  ready_.store(false);
  removeNativeHooks();
  states_.clear();
}
} // namespace mr

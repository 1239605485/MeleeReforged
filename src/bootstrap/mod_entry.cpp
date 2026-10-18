#include "interop/game_runtime.hpp"
#include "mod_core.h"
#include <cstdio>
#include <dlfcn.h>
#include <mutex>
#include <string>
extern "C" {
__attribute__((visibility("default"))) void (*mod_logger_write)(int,
                                                                const char *,
                                                                const char *,
                                                                ...) = nullptr;
}
namespace {
std::string logPath;
std::mutex logMutex;
void init(kernel_mod_handle_t *h) {
  if (h && h->private_dir)
    logPath = std::string(h->private_dir) + "/melee_reforged_runtime.log";
  // KernelLoader's void cleanup callback cannot veto dlclose. Keep this code
  // resident so an in-flight or failed-to-remove native callback cannot point
  // into unmapped memory. A process restart is required to replace this version.
  static void *pin = nullptr;
  if (!pin) {
    pin = dlopen("libMeleeReforged.android.arm64.so", RTLD_NOW | RTLD_NOLOAD);
    if (pin && dlsym(pin, "create_kernel_mod") !=
                   reinterpret_cast<void *>(&create_kernel_mod)) {
      dlclose(pin);
      pin = nullptr;
    }
  }
  if (!pin) {
    mr::log("Cannot pin plugin lifetime; hooks disabled for unload safety.");
    return;
  }
  mr::runtime().start();
}
void cleanup(kernel_mod_handle_t *) { mr::runtime().stop(); }
kernel_mod_info_t info{"liuxin.melee.reforged", 1, 1, "0.1.0"};
kernel_mod_info_t *getInfo() { return &info; }
kernel_mod_ops_t ops{init, cleanup, getInfo};
} // namespace
namespace mr {
void log(const char *message) {
  if (mod_logger_write)
    mod_logger_write(2, "MeleeReforged", "%s", message ? message : "unknown");
  std::lock_guard<std::mutex> guard(logMutex);
  if (!logPath.empty()) {
    if (auto f = std::fopen(logPath.c_str(), "a")) {
      std::fprintf(f, "%s\n", message ? message : "unknown");
      std::fclose(f);
    }
  }
}
} // namespace mr
extern "C" __attribute__((visibility("default"))) kernel_mod_ops_t *
create_kernel_mod() {
  return &ops;
}

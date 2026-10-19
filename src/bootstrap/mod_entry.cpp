#include "interop/game_runtime.hpp"
#include "mod_core.h"
#include <cstdio>
#include <mutex>
#include <string>
extern "C" {
// 1 requests residency; matching KernelLoader acknowledges with 2.
__attribute__((visibility("default"))) int kernel_mod_residency_v1 = 1;
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
  mr::log("MeleeReforged initialization entered.");
  if (kernel_mod_residency_v1 != 2) {
    mr::log("KernelLoader residency handshake missing; install the matching loader and restart.");
    return;
  }
  mr::runtime().start();
}
void cleanup(kernel_mod_handle_t *) { mr::runtime().stop(); }
kernel_mod_info_t info{"liuxin.melee.reforged", 11, 1, "0.7.0"};
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

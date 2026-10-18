#include "interop/hook_set.hpp"
#include "interop/runtime_api.hpp"
#include <cstdio>
#include <cstdlib>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      std::fprintf(stderr, "line %d: %s\n", __LINE__, #x);                     \
      std::exit(1);                                                            \
    }                                                                          \
  } while (0)
static int installs, removed;
static int install(void *, void *, void **original) {
  *original = reinterpret_cast<void *>(9);
  return ++installs == 3 ? -1 : 0;
}
static int uninstall(void *h) {
  removed += static_cast<int>(reinterpret_cast<std::uintptr_t>(h));
  return 0;
}
static int failRemove(void *) { return -1; }
int main() {
  mr::RuntimeApi api;
  CHECK(!api.resolve().compatible);
  void *original{};
  mr::HookSet hooks(install, uninstall);
  mr::HookRequest r{reinterpret_cast<void *>(1), reinterpret_cast<void *>(8),
                    &original};
  CHECK(hooks.install(r));
  r.target = reinterpret_cast<void *>(2);
  CHECK(hooks.install(r));
  r.target = reinterpret_cast<void *>(3);
  CHECK(!hooks.install(r));
  CHECK(removed == 3);
  CHECK(hooks.reset());
  CHECK(removed == 3);
  installs = 0;
  mr::HookSet broken(install, failRemove);
  CHECK(broken.install(r));
  CHECK(!broken.reset());
  CHECK(broken.count() == 1);
  CHECK(!broken.install(r));
  std::puts("interop checks passed");
}

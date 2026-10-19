#include "interop/game_runtime.hpp"
#include "mod_core.h"
#include <cassert>
extern "C" int kernel_mod_residency_v1;
static int starts, stops;
namespace mr {
GameRuntime &runtime() { static GameRuntime instance; return instance; }
bool GameRuntime::start() { ++starts; return true; }
void GameRuntime::startWeaponApi(kernel_mod_handle_t*) {}
void GameRuntime::stop() { ++stops; }
}
int main() {
 auto ops=create_kernel_mod(); kernel_mod_handle_t handle{};
 kernel_mod_residency_v1=1; ops->init_mod(&handle); assert(starts==0);
 kernel_mod_residency_v1=2; ops->init_mod(&handle); assert(starts==1);
 ops->cleanup_mod(&handle); assert(stops==1);
}

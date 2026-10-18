#pragma once
#include "runtime_api.hpp"
namespace mr {
bool installNativeHooks(RuntimeApi &);
bool removeNativeHooks();
} // namespace mr

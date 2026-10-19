#pragma once
#include "runtime_api.hpp"
namespace mr {
// Created once on the existing held-item draw thread; retained across worlds.
Handle effectAtlas(RuntimeApi &,bool katana=false);
}

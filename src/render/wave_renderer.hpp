#pragma once
#include "combat/sword_wave.hpp"
#include "swing_renderer.hpp"
namespace mr {
class WaveRenderer {
 public:static DrawCommands build(const SwordWave &wave);
};
}

#pragma once
#include "attack_types.hpp"
namespace mr {
// One bounded ground impulse per attack. Vanilla simulation still owns position
// and tile collision; never teleport through walls or override airborne motion.
inline Vec2 stepVelocity(const AttackPose &p,Vec2 velocity,bool allowed) {
 if(!allowed || !p.damaging || std::abs(velocity.y)>.01f ||
    !std::isfinite(velocity.x) || !std::isfinite(velocity.y)) return velocity;
 float target=p.charged?2.5f:(p.combo==2?2.1f:1.6f);
 float forward=velocity.x*p.direction;
 if(forward<-.1f || forward>=target) return velocity;
 return {target*p.direction,velocity.y};
}
}

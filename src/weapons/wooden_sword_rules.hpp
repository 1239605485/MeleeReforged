#pragma once
#include <algorithm>
#include <cmath>
#include <limits>
namespace mr {
struct AttackPower {float damage{1},knockback{1};};
inline AttackPower woodenMeleePower(bool charged,int combo){
 if(!charged)return {};
 constexpr AttackPower heavy[]={{2.5f,1.8f},{1.3f,1.f},{1.5f,1.f}};
 return heavy[std::clamp(combo,0,2)];
}
inline constexpr AttackPower woodenWavePower{1.2f,2.f};
inline int scaleDamage(int damage,float multiplier){
 if(damage<=0 || !std::isfinite(multiplier) || multiplier<=0)return 0;
 return static_cast<int>(std::min(double(std::numeric_limits<int>::max()),
                                 std::max(1.,std::floor(double(damage)*multiplier+.5))));
}
}

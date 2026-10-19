#pragma once
#include "combat/attack_types.hpp"
#include "weapons/weapon_registry.hpp"
#include <algorithm>
namespace mr {
struct KatanaFan {bool visible{};int frame{},row{},effects{};float rotation{},scale{},opacity{};};
inline KatanaFan katanaFan(const AttackPose &p,float cycle){
 KatanaFan f{};if(!p.active || p.charging || p.itemId!=2273)return f;
 const auto &s=p.charged?katana().heavy[0]:katana().swings[0];
 if(p.progress<s.windup)return f;
 float cut=(p.progress-s.windup)/s.swing;
 float tailAge=std::max(0.f,p.progress-s.windup-s.swing)*cycle*(p.charged?katana().heavyTiming:1.f);
 if(tailAge>=.14f)return f;
 f.visible=true;f.frame=std::min(7,int(std::clamp(cut,0.f,1.f)*8));f.row=p.charged?128:0;
 float a=cut<=1?p.angle:s.endAngle;
 f.rotation=std::atan2(std::sin(a)*p.gravity,std::cos(a)*p.direction);
 f.effects=p.direction*p.gravity<0?2:0;
 f.scale=p.bladeLength*(p.charged?1.6f:1.3f)/54.f;
 f.opacity=std::min(1.f,cut*8.f)*(1-tailAge/.14f);return f;
}
}

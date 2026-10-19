#pragma once
#include "draw_bridge.hpp"
#include "combat/player_states.hpp"
#include "weapons/weapon_registry.hpp"
#include <algorithm>
namespace mr {
// Consumes simulation snapshots only; repeated render passes spawn nothing.
inline void drawKatanaEffects(RuntimeApi &api,Handle ctx,Handle atlas,const PlayerState &s,Vec2 screen){
 if(!atlas)return;
 const auto &p=s.current;
 auto stamp=[&](int cell,Vec2 pos,float angle,Vec2 scale,float alpha){
  if(alpha<=0)return;
  auto slot=invoke<Handle>(api.addDraw,ctx);if(!slot)return;
  invoke<void>(api.setDrawRect,slot,atlas,pos-screen,NativeRect{cell*64,0,64,64},
   toNativeColor({170,215,255,static_cast<unsigned char>(std::clamp(alpha,0.f,1.f)*255)}),
   angle,Vec2{32,32},scale,0,0);
 };
 auto worldAngle=[&](float a){return std::atan2(std::sin(a)*p.gravity,std::cos(a)*p.direction);};
 if(p.charging){
  float breathe=.7f+.3f*std::sin(float(p.tick%120)*.16f);
  stamp(3,p.grip+(p.tip-p.grip)*.72f,0,{.25f,.25f},p.chargeLevel*.55f*breathe);
  stamp(2,p.tip,worldAngle(p.angle),{.16f,.16f},p.chargeLevel*.7f*breathe);
  return;
 }
 const auto &profile=p.charged?katana().heavy[p.combo]:katana().swings[p.combo];
 float age=(p.progress-profile.windup)*s.cycleSeconds*(p.charged?katana().heavyTiming:katana().normalTiming[p.combo]);
 if(age>=0 && age<.15f){
  float fade=std::min(age/.025f,1.f)*std::clamp(1-age/.15f,0.f,1.f);
  auto center=p.grip+(p.tip-p.grip)*.65f;
  float angle=worldAngle(p.charged?-.12f:p.angle);
  float span=p.bladeLength*(p.charged?1.75f:1.25f)/60.f;
  stamp(p.charged?1:0,center,angle+(p.charged?0.f:.785398f),{span,span},fade*.8f);
  if(p.charged && age>=.05f)stamp(1,center+Vec2{0,3*p.gravity},angle,{span,.65f*span},fade*.27f);
 }
 auto now=invokeStatic<unsigned>(api.tick);
 for(const auto &hit:s.impacts){
  if(!hit.active || now<hit.tick || now-hit.tick>8)continue;
  float fade=1-float(now-hit.tick)/9.f;
  stamp(2,hit.position,.35f,{.28f+.12f*fade,.28f+.12f*fade},fade);
 }
}
}

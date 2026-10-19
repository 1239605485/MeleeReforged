#pragma once
#include "draw_bridge.hpp"
#include "combat/player_states.hpp"
#include "render/katana_trail.hpp"
#include <algorithm>
namespace mr {
// All positions come from simulation blade samples. No fixed slash decal,
// render-driven lifetime, or extra gameplay projectile.
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
 if(p.charging){
  // Compact inward-moving glints reveal the blade and charge direction.
  Vec2 blade=p.tip-p.grip;
  float pulse=.75f+.25f*std::sin(float(p.tick%120)*.16f);
  stamp(3,p.grip+blade*.72f,0,{.35f,.35f},p.chargeLevel*.6f*pulse);
  for(int i=0;i<4;++i){
   float t=std::fmod(float(p.tick%120)*.035f+i*.25f,1.f);
   Vec2 at=p.grip+blade*(.35f+.5f*t)+Vec2{(1-t)*8*p.direction,(1-t)*-12*p.gravity};
   stamp(2,at,0,{.11f,.11f},p.chargeLevel*t*.65f);
  }
  if(p.chargeLevel>=.999f)stamp(2,p.tip,0,{.24f,.24f},.55f*pulse);
  return;
 }
 auto ribbon=katanaTrail(s.previous,p,s.trail);
 for(std::size_t i=0;i<ribbon.count;++i){
  const auto &v=ribbon.segments[i];Vec2 d=v.to-v.from;float span=length(d);
  if(span<.02f || span>p.bladeLength*2)continue;
  auto center=(v.from+v.to)*.5f;float a=std::atan2(d.y,d.x);
  // Soft outer strip and sharp inner strip share exactly the same trajectory.
  stamp(0,center,a,{(span+3)/60.f,p.charged?.95f:.6f},v.fade*(p.charged?.65f:.45f));
  stamp(1,center,a,{(span+2)/60.f,p.charged?1.35f:.8f},v.fade*.92f);
 }
 auto now=invokeStatic<unsigned>(api.tick);
 for(const auto &hit:s.impacts){
  if(!hit.active || now<hit.tick || now-hit.tick>8)continue;
  float fade=1-float(now-hit.tick)/9.f;
  stamp(2,hit.position,.35f,{.28f+.12f*fade,.28f+.12f*fade},fade);
 }
}
}

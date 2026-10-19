#pragma once
#include "draw_bridge.hpp"
#include "combat/player_states.hpp"
#include "render/katana_fan.hpp"
#include <algorithm>
namespace mr {
inline void katanaStamp(RuntimeApi &api,Handle ctx,Handle atlas,NativeRect rect,Vec2 pos,float angle,Vec2 origin,Vec2 scale,Color color,int effects=0){
 if(!atlas)return;
 auto slot=invoke<Handle>(api.addDraw,ctx);
 if(!slot)return;
 invoke<void>(api.setDrawRect,slot,atlas,pos,rect,toNativeColor(color),angle,origin,scale,effects,0);
}
inline void drawKatanaSheath(RuntimeApi &api,Handle ctx,Handle atlas,Vec2 grip,Vec2 screen,int direction,float gravity,float blade){
 float angle=std::atan2(std::sin(2.35f)*gravity,std::cos(2.35f)*direction);
 Vec2 axis{std::cos(angle),std::sin(angle)};
 katanaStamp(api,ctx,atlas,{256,256,64,64},grip+axis*(blade*.18f)-screen,angle,{2,32},{blade*.92f/60.f,.70f},{255,255,255,255},direction*gravity<0?2:0);
}
inline void drawKatanaEffects(RuntimeApi &api,Handle ctx,Handle atlas,const PlayerState &s,Vec2 screen){
 if(!atlas)return;
 const auto &p=s.current;
 auto stamp=[&](int cell,Vec2 pos,float angle,Vec2 scale,float alpha){
  katanaStamp(api,ctx,atlas,{cell*64,256,64,64},pos-screen,angle,{32,32},scale,
   {175,220,255,static_cast<unsigned char>(std::clamp(alpha,0.f,1.f)*255)});
 };
 if(p.charging){
  float pulse=.75f+.25f*std::sin(float(p.tick%120)*.16f);
  stamp(3,p.grip,0,{.35f+.15f*p.chargeLevel,.35f+.15f*p.chargeLevel},p.chargeLevel*.8f*pulse);
  for(int i=0;i<6;++i){float t=std::fmod(float(p.tick%120)*.035f+i/6.f,1.f);float a=i*2.39996f;
   Vec2 pos=p.grip+Vec2{std::cos(a),std::sin(a)}*(6+(1-t)*17);
   stamp(2,pos,0,{.12f,.12f},p.chargeLevel*t*.75f);
  }
  if(p.chargeLevel>=.999f)stamp(2,p.grip,0,{.35f,.35f},.7f*pulse);
  return;
 }
 auto f=katanaFan(p,s.cycleSeconds);
 if(f.visible){
  Vec2 pivot=p.grip;
  if(p.phase==Phase::Recovery && s.trail.size())pivot=s.trail[s.trail.size()-1].pose.grip;
  katanaStamp(api,ctx,atlas,{f.frame*128,f.row,128,128},pivot-screen,f.rotation,{64,64},{f.scale,f.scale},
   {170,215,255,static_cast<unsigned char>(f.opacity*245)},f.effects);
 }
 auto now=invokeStatic<unsigned>(api.tick);
 for(const auto &hit:s.impacts){if(!hit.active || now<hit.tick || now-hit.tick>8)continue;
  float fade=1-float(now-hit.tick)/9.f;stamp(2,hit.position,.35f,{.4f+.15f*fade,.4f+.15f*fade},fade);
 }
}
}

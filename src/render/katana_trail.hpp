#pragma once
#include "trail_buffer.hpp"
#include "collision/damage_sweep.hpp"
namespace mr {
struct KatanaRibbonSegment {Vec2 from,to;float fade;};
struct KatanaRibbon {std::array<KatanaRibbonSegment,32> segments{};std::size_t count{};};
inline KatanaRibbon katanaTrail(const AttackPose &previous,const AttackPose &current,const TrailBuffer &history){
 KatanaRibbon out;
 if(!current.active || current.charging || current.itemId!=2273)return out;
 auto edge=[](BladeSegment p){return p.grip+(p.tip-p.grip)*.94f;};
 auto sweep=collectDamageSweep(previous,current);
 if(sweep.attackId==current.attackId)
  for(std::size_t i=1;i<sweep.count && out.count<out.segments.size();++i)
   out.segments[out.count++]={edge(sweep.segments[i-1]),edge(sweep.segments[i]),1.f};
 // Historical blade positions create the fading tail; rendering never advances it.
 for(std::size_t i=history.size();i>1 && out.count<out.segments.size();--i){
  const auto &b=history[i-1].pose;const auto &a=history[i-2].pose;
  if(a.attackId!=current.attackId || b.attackId!=current.attackId || b.tick>=current.tick || current.tick-b.tick>6)continue;
  float fade=1.f-float(current.tick-b.tick)/7.f;
  out.segments[out.count++]={edge({a.grip,a.tip}),edge({b.grip,b.tip}),fade};
 }
 return out;
}
}

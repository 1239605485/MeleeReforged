#pragma once
#include "attack_types.hpp"
#include <array>
#include <algorithm>
namespace mr {
struct WaveMotion {Vec2 from{},to{};};
struct WaveTrace {Vec2 end{};bool blocked{};};
template<class Blocked> WaveTrace traceWavePath(Vec2 from,Vec2 to,Blocked blocked){
 float distance=length(to-from);
 if(!std::isfinite(distance) || distance>256)return {from,true};
 int steps=std::max(1,int(std::ceil(distance/2.f)));
 Vec2 clear=from;
 for(int i=0;i<=steps;++i){
  Vec2 point=from+(to-from)*(float(i)/steps);
  if(blocked(point))return {clear,true};
  clear=point;
 }
 return {to,false};
}
struct SwordWave {
 bool active{};Vec2 position{},previous{};int direction{1};float gravity{1};
 float age{},distance{},knockback{};int damage{};
 std::array<std::uint64_t,200> hitGeneration{};
 std::array<bool,200> hit{};
 void launch(Vec2 origin,int facing,float grav,int power,float knock){
  *this={};active=true;position=previous=origin;direction=facing<0?-1:1;
  gravity=grav<0?-1.f:1.f;damage=power;knockback=knock;
 }
 WaveMotion step(float dt){
  WaveMotion motion{position,position};
  if(!active)return motion;
  if(!std::isfinite(dt) || dt<0){active=false;return motion;}
  previous=position;
  float travel=std::min(96.f-distance,240.f*dt);
  position.x+=direction*travel;distance+=travel;age+=dt;
  motion.to=position;
  if(distance>=96.f-1e-4f || age>=.45f)active=false;
  return motion;
 }
 bool canHit(int slot,std::uint64_t generation)const{
  return slot>=0 && slot<200 && (!hit[slot] || hitGeneration[slot]!=generation);
 }
 void markHit(int slot,std::uint64_t generation){
  if(slot>=0 && slot<200){hit[slot]=true;hitGeneration[slot]=generation;}
 }
};
}

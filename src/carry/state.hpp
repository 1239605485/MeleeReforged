#pragma once
#include "combat/carry_state.hpp"
#include "registry.hpp"
#include <cstdint>
namespace mr {
struct CarryPresentationState {
 int itemId{};CarryProfile profile{};CarryState timing{};
 std::uint64_t lastTick{~std::uint64_t(0)};
 bool eligible{},usingItem{},hasUsePose{};
 Vec2 useOffset{};float useAngle{};
 int useDirection{1};float useGravity{1};
 std::uintptr_t backContext{};bool backDrawn{};
 void observe(int id,bool supported,bool usingNow,std::uint64_t tick,float dt){
  if(id!=itemId){*this={};itemId=id;}
  if(lastTick==tick)return;
  lastTick=tick;eligible=supported;usingItem=usingNow;
  timing.advance(usingNow,dt);
 }
 float amount()const{return timing.amount();}
 bool visible()const{return eligible && !usingItem;}
};
}

#pragma once
#include "attack_button.hpp"
namespace mr {
// Android: attack alone repeats normal cuts; down+attack at press selects heavy.
// Selection is latched until attack release, so moving the stick never upgrades
// a running normal attack or fires a charged attack accidentally.
class KatanaInput {
 bool held_{},heavy_{};float charge_{};
public:
 void reset(){held_=heavy_=false;charge_=0;}
 ButtonIntent advance(bool attack,bool down,float dt,bool canCharge){
  if(!std::isfinite(dt) || dt<0){reset();return {};}
  if(attack){
   if(!held_){heavy_=down;charge_=0;}
   else if(heavy_ && canCharge)charge_=std::min(.55f,charge_+dt);
   held_=true;
   return {heavy_,!heavy_,false,charge_/.55f};
  }
  bool release=held_ && heavy_ && charge_>=.55f-1e-5f;
  reset();return {false,false,release,0};
 }
};
}

#pragma once
#include "attack_types.hpp"
#include <algorithm>
namespace mr {
class CarryState {
 float idle_{1.f};
 public:
 void advance(bool active,float dt){
  if(active)idle_=0;
  else if(std::isfinite(dt) && dt>0)idle_=std::min(1.f,idle_+dt);
 }
 float amount()const{return std::clamp((idle_-.25f)/.2f,0.f,1.f);}
 bool ready()const{return amount()<1;}
};
inline Vec2 waistGrip(Vec2 position,int width,int height,int direction,float gravity){
 return position+Vec2{width*.5f+direction*6.f,
   height*.5f-4.f*gravity};
}
inline float waistAngle(int direction,float gravity){
 return std::atan2(std::sin(2.35f)*gravity,std::cos(2.35f)*direction);
}
}

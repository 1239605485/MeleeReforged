#pragma once
#include "registry.hpp"
namespace mr {
struct CarriedPose {Vec2 position{};float angle{};bool back{};int effects{};};
inline CarriedPose carriedPose(const CarryProfile &profile,Vec2 position,int width,int height,
                               int direction,float gravity,float sway,bool mounted){
 Vec2 offset=profile.offset;float angle=profile.angle;
 bool back=profile.back;
 if(mounted && back){offset={8,1};angle=1.25f;back=false;}
 offset.y+=sway;
 return {position+Vec2{width*.5f+offset.x*direction,height*.5f+offset.y*gravity},
  std::atan2(std::sin(angle)*gravity,std::cos(angle)*direction),back,
  direction*gravity<0?2:0};
}
}

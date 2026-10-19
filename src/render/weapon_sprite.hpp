#pragma once
#include "combat/attack_types.hpp"
namespace mr {
struct WeaponSprite {Vec2 origin,sourceTip;float rotation{},scale{};int effects{};};
inline WeaponSprite weaponSprite(int item,float width,float height,Vec2 grip,Vec2 tip,int direction,float gravity){
 // Katana reference canvas 48x54: grip at (8,46), tip at (45,2).
 // Reflect the source pivot and blade axis together with the texture UVs.
 Vec2 origin=item==2273?Vec2{width*8/48,height*46/54}:Vec2{2,height-2};
 Vec2 sourceTip=item==2273?Vec2{width*45/48,height*2/54}:Vec2{width-2,2};
 int effects=item==2273 && direction*gravity<0?2:0;
 if(effects){origin.y=height-origin.y;sourceTip.y=height-sourceTip.y;}
 auto axis=sourceTip-origin;
 return {origin,sourceTip,std::atan2(tip.y-grip.y,tip.x-grip.x)-std::atan2(axis.y,axis.x),length(tip-grip)/length(axis),effects};
}
}

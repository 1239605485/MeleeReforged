#include "registry.hpp"
#include <algorithm>
namespace mr {
bool isVanillaWhip(int id){
 // Verified against the supplied 1.4.5 ItemID constants, including the new whips.
 constexpr int ids[]={4672,4678,4679,4680,4911,4912,4913,4914,5074,5473,5474,5475,5476,5477,5478,5479,5480};
 for(int known:ids)if(id==known)return true;
 return false;
}
CarryProfile carryProfile(const CarryTraits &t,int width,int height){
 if(t.itemId<=0 || width<=0 || height<=0 || width>4096 || height>4096 || t.accessory || t.consumable)return {};
 CarryProfile p;int span=std::max(width,height);
 int tools=int(t.pick>0)+int(t.axe>0)+int(t.hammer>0);
 if(tools){
  p.kind=tools>1?CarryKind::MultiTool:(t.pick>0?CarryKind::Pickaxe:(t.axe>0?CarryKind::Axe:CarryKind::Hammer));
  bool large=span>48;
  p.back=large || p.kind==CarryKind::Pickaxe || p.kind==CarryKind::MultiTool;
  p.offset=large?Vec2{-4,-6}:Vec2{5,0};p.angle=large?1.15f:2.35f;
  p.sourceAngle=t.noUseGraphic && t.useStyle==5?0.f:-.785398f;
  p.maxSpan=large?58.f:42.f;return p;
 }
 if(isVanillaWhip(t.itemId)){
  p.kind=CarryKind::Whip;p.offset={8,3};p.angle=.2f;p.maxSpan=20;
  switch(t.itemId){case 4913:case 5477:p.rope={95,170,75};break;
   case 4911:case 5478:p.rope={110,205,240};break;
   case 4912:case 5476:p.rope={240,125,55};break;
   case 4678:p.rope={225,190,95};break;case 4679:case 4680:case 5474:p.rope={155,100,185};break;
   case 4914:case 5479:case 5480:p.rope={225,145,225};break;case 5074:case 5473:p.rope={210,205,185};break;default:break;}
  return p;
 }
 if(t.damage<=0 && !t.ranged && !t.magic && !t.summon)return {};
 if(t.ranged){
  if(t.arrow){p.kind=CarryKind::Bow;p.offset={-5,-6};p.angle=1.25f;p.sourceAngle=1.570796f;p.maxSpan=54;p.back=true;}
  else if(t.useStyle==5){
   p.kind=span<=36?CarryKind::Handgun:(span>=64?CarryKind::HeavyGun:CarryKind::LongGun);
   p.back=p.kind!=CarryKind::Handgun;p.offset=p.back?Vec2{-5,-6}:Vec2{8,5};p.angle=p.back?1.05f:1.35f;p.maxSpan=p.back?62.f:32.f;
  }
  return p;
 }
 if(t.magic || t.summon){
  p.kind=t.magic && !t.staff?CarryKind::Book:CarryKind::Staff;
  p.back=p.kind==CarryKind::Staff && span>36;p.offset=p.back?Vec2{-5,-8}:Vec2{8,2};
  p.angle=p.kind==CarryKind::Book?.15f:1.2f;p.sourceAngle=p.kind==CarryKind::Book?0:-.785398f;p.maxSpan=p.back?58.f:32.f;return p;
 }
 if(t.melee){
  if(t.noUseGraphic && t.channel)return {};
  p.kind=t.noUseGraphic && t.useStyle==5?CarryKind::Spear:(span>52?CarryKind::GreatBlade:CarryKind::Blade);
  bool large=p.kind!=CarryKind::Blade;p.back=true;p.offset=large?Vec2{-4,-7}:Vec2{5,0};
  p.angle=p.kind==CarryKind::Spear?1.38f:(large?1.1f:2.35f);p.sourceAngle=-.785398f;p.maxSpan=large?62.f:44.f;return p;
 }
 return {};
}
}

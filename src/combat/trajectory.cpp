#include "trajectory.hpp"
#include <algorithm>
namespace mr {
namespace {
float smooth(float t) { t=std::clamp(t,0.f,1.f);return t*t*(3-2*t); }
float mix(float a,float b,float t) {return a+(b-a)*t;}
struct Key {float blade,arm,lean,drop,shift,back;};
Key blend(Key a,Key b,float t) {
 return {mix(a.blade,b.blade,t),mix(a.arm,b.arm,t),mix(a.lean,b.lean,t),
         mix(a.drop,b.drop,t),mix(a.shift,b.shift,t),mix(a.back,b.back,t)};
}
}
AttackPose evaluateKatanaPose(const WeaponProfile &,int,float,Vec2,int,float,float,bool);
AttackPose evaluatePose(const WeaponProfile &weapon,int combo,float u,Vec2 grip,
                        int direction,float gravity,float bladeLength,bool charged) {
  if(weapon.itemId==2273)return evaluateKatanaPose(weapon,combo,u,grip,direction,gravity,bladeLength,charged);
  AttackPose p{};
  p.active=true;p.combo=std::clamp(combo,0,2);p.progress=std::clamp(u,0.f,1.f);
  p.direction=direction<0?-1:1;p.gravity=gravity<0?-1.f:1.f;
  p.bladeLength=bladeLength;p.charged=charged;
  const auto &s=charged?weapon.heavy[p.combo]:weapon.swings[p.combo];
  // Each endpoint is the next move's entry: the sword does not reset between blows.
  constexpr Key rest{-.9f,.55f,0,0,0,1.5f};
  constexpr Key normalEntry[]={rest,{.92f,1.0f,.30f,3,3,2.3f},{-1.85f,-.7f,-.12f,1,-1,1.8f},rest};
  constexpr Key heavyEntry[]={{-2.5f,-1.7f,-.25f,4,-3,2.1f},{1.15f,.9f,.38f,5,4,2.7f},{-.35f,.2f,.22f,3,3,2.4f},rest};
  constexpr float normalArms[]={-1.4f,1.25f,2.8f};
  constexpr float heavyArms[]={-1.65f,1.15f,1.2f};
  const auto *entry=charged?heavyEntry:normalEntry;
  Key from=entry[p.combo];
  Key wind{s.startAngle,charged?heavyArms[p.combo]:normalArms[p.combo],
           charged?-.25f:-.16f,charged?4.f:2.f,charged?-3.f:-1.5f,2.f};
  if(!charged && p.combo==2) wind={s.startAngle,2.8f,-.18f,2,-2,2.3f};
  Key cut{s.endAngle,charged?.45f:.55f,charged?.38f:.30f,
          charged?5.f:3.f,charged?4.f:3.f,2.7f};
  if(p.combo==1) cut.arm=-.7f;
  if(charged && p.combo==2) {cut.arm=-1.1f;cut.lean=-.18f;cut.drop=0;}
  if(!charged && p.combo==2) {cut.arm=.05f;cut.lean=.35f;cut.drop=3;cut.shift=4;}
  Key key{};
  if(p.progress<s.windup) {
    p.phase=Phase::Windup;key=blend(from,wind,smooth(p.progress/s.windup));
  } else if(p.progress<s.windup+s.swing) {
    p.phase=Phase::Swing;p.damaging=true;
    // Fast acceleration, a slower follow-through; distinct from a constant rotation.
    float t=smooth((p.progress-s.windup)/s.swing);
    key=blend(wind,cut,t);
  } else {
    p.phase=Phase::Recovery;
    float t=smooth((p.progress-s.windup-s.swing)/s.recovery);
    key=blend(cut,entry[p.combo+1],t);
  }
  p.angle=key.blade;p.armAngle=key.arm;p.backArmAngle=key.back;
  p.bodyLean=key.lean*p.direction*p.gravity;p.bodyDrop=key.drop*p.gravity;
  p.bodyShift=key.shift*p.direction;
  p.legLean=-key.lean*.35f*p.direction*p.gravity;
  p.legShift=-key.shift*.45f*p.direction;
  p.handOffset={std::cos(key.arm)*12.f*p.direction,std::sin(key.arm)*12.f*p.gravity};
  p.grip=grip+p.handOffset;
  p.tip=p.grip+Vec2{std::cos(p.angle)*bladeLength*p.direction,
                   std::sin(p.angle)*bladeLength*p.gravity};
  return p;
}
BladeSegment sampleBlade(const AttackPose &p,float n) {
 return {p.grip,p.grip+Vec2{std::cos(p.angle)*std::min(n,p.bladeLength)*p.direction,
                           std::sin(p.angle)*std::min(n,p.bladeLength)*p.gravity}};
}
} // namespace mr

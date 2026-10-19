#include "trajectory.hpp"
#include <algorithm>
namespace mr {
AttackPose evaluateKatanaPose(const WeaponProfile &w,int combo,float u,Vec2 anchor,int direction,float gravity,float blade,bool heavy){
 AttackPose p{};p.itemId=w.itemId;p.active=true;p.combo=std::clamp(combo,0,2);
 p.direction=direction<0?-1:1;p.gravity=gravity<0?-1.f:1.f;p.bladeLength=blade;
 p.charged=heavy;p.progress=std::clamp(u,0.f,1.f);
 const auto &s=heavy?w.heavy[p.combo]:w.swings[p.combo];
 struct Key {float angle,arm,lean,drop,shift;};
 constexpr Key entry[]={{2.35f,1.8f,0,0,0},{.85f,.70f,.12f,1,1},{-1.65f,-1.f,-.10f,0,-1},{2.35f,1.8f,0,0,0}};
 constexpr float windArms[]={-1.05f,1.05f,-.85f};
 Key from=heavy?Key{2.35f,2.45f,-.12f,3,-2}:entry[p.combo];
 Key wind{s.startAngle,heavy?2.45f:windArms[p.combo],heavy?-.12f:-.08f,heavy?3.f:1.f,heavy?-2.f:-1.f};
 Key cut=heavy?Key{s.endAngle,-.12f,.23f,2,2}:entry[p.combo+1];cut.angle=s.endAngle;
 if(!heavy && p.combo==2)cut={s.endAngle,.12f,.15f,1,1};
 auto smooth=[](float t){t=std::clamp(t,0.f,1.f);return t*t*(3-2*t);};
 auto mix=[](Key a,Key b,float t){return Key{a.angle+(b.angle-a.angle)*t,a.arm+(b.arm-a.arm)*t,a.lean+(b.lean-a.lean)*t,a.drop+(b.drop-a.drop)*t,a.shift+(b.shift-a.shift)*t};};
 Key k{};
 if(p.progress<s.windup){p.phase=Phase::Windup;k=mix(from,wind,smooth(p.progress/s.windup));}
 else if(p.progress<s.windup+s.swing){p.phase=Phase::Swing;p.damaging=true;float t=(p.progress-s.windup)/s.swing;t=1-std::pow(1-t,3.f);k=mix(wind,cut,t);}
 else {p.phase=Phase::Recovery;float t=smooth((p.progress-s.windup-s.swing)/s.recovery);k=mix(cut,heavy?entry[0]:entry[p.combo+1],t);}
 p.angle=k.angle;p.armAngle=k.arm;p.backArmAngle=heavy?2.55f:2.1f;
 p.bodyLean=k.lean*p.direction*p.gravity;p.bodyDrop=k.drop*p.gravity;p.bodyShift=k.shift*p.direction;
 p.legLean=-p.bodyLean*.2f;p.legShift=-p.bodyShift*.25f;
 p.handOffset={std::cos(k.arm)*12*p.direction,std::sin(k.arm)*12*p.gravity};
 p.grip=anchor+p.handOffset;p.tip=p.grip+Vec2{std::cos(p.angle)*blade*p.direction,std::sin(p.angle)*blade*p.gravity};return p;
}
}

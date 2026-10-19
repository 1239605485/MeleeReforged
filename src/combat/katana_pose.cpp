#include "trajectory.hpp"
#include <algorithm>
namespace mr {
AttackPose evaluateKatanaPose(const WeaponProfile &w,int,float u,Vec2 anchor,int direction,float gravity,float blade,bool heavy){
 AttackPose p{};p.itemId=w.itemId;p.active=true;p.combo=0;
 p.direction=direction<0?-1:1;p.gravity=gravity<0?-1.f:1.f;p.bladeLength=blade;
 p.charged=heavy;p.progress=std::clamp(u,0.f,1.f);
 const auto &s=heavy?w.heavy[0]:w.swings[0];
 struct Key {float angle,arm,lean,drop,shift;};
 Key ready{2.35f,1.8f,0,0,0},wind{2.35f,1.8f,heavy?-.15f:-.08f,heavy?3.f:1.f,heavy?-2.f:-1.f};
 Key cut{s.endAngle,-.08f,heavy?.24f:.13f,heavy?2.f:1.f,heavy?2.f:1.f};
 auto smooth=[](float t){t=std::clamp(t,0.f,1.f);return t*t*(3-2*t);};
 auto mix=[](Key a,Key b,float t){return Key{a.angle+(b.angle-a.angle)*t,a.arm+(b.arm-a.arm)*t,a.lean+(b.lean-a.lean)*t,a.drop+(b.drop-a.drop)*t,a.shift+(b.shift-a.shift)*t};};
 Key k{};
 if(p.progress<s.windup){p.phase=Phase::Windup;k=mix(ready,wind,smooth(p.progress/s.windup));}
 else if(p.progress<s.windup+s.swing){p.phase=Phase::Swing;p.damaging=true;float t=(p.progress-s.windup)/s.swing;t=1-std::pow(1-t,3.f);k=mix(wind,cut,t);}
 else {p.phase=Phase::Recovery;float t=(p.progress-s.windup-s.swing)/s.recovery;
  // Hold the finishing pose, then smoothly return to the sheath.
  k=mix(cut,ready,smooth((t-.22f)/.78f));}
 p.angle=k.angle;p.armAngle=k.arm;p.backArmAngle=heavy?2.4f:2.1f;
 p.bodyLean=k.lean*p.direction*p.gravity;p.bodyDrop=k.drop*p.gravity;p.bodyShift=k.shift*p.direction;
 p.legLean=-p.bodyLean*.2f;p.legShift=-p.bodyShift*.25f;
 p.handOffset={std::cos(k.arm)*12*p.direction,std::sin(k.arm)*12*p.gravity};
 p.grip=anchor+p.handOffset;p.tip=p.grip+Vec2{std::cos(p.angle)*blade*p.direction,std::sin(p.angle)*blade*p.gravity};return p;
}
}

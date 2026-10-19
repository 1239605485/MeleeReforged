#include "trajectory.hpp"
#include <algorithm>
namespace mr {
AttackPose evaluateKatanaPose(const WeaponProfile &w,int combo,float u,Vec2 anchor,int direction,float gravity,float blade,bool heavy){
 AttackPose p{};p.itemId=w.itemId;p.active=true;p.combo=std::clamp(combo,0,2);
 p.direction=direction<0?-1:1;p.gravity=gravity<0?-1.f:1.f;p.bladeLength=blade;
 p.charged=heavy;p.progress=std::clamp(u,0.f,1.f);
 const auto &s=heavy?w.heavy[p.combo]:w.swings[p.combo];
 // Recovery ends at the next move's entry. Heavy always returns to low guard.
 constexpr float entry[]={-.85f,.60f,-.95f,-.85f};
 float start=heavy?s.startAngle:entry[p.combo],end=heavy?entry[0]:entry[p.combo+1];
 auto smooth=[](float t){t=std::clamp(t,0.f,1.f);return t*t*(3-2*t);};
 if(p.progress<s.windup){p.phase=Phase::Windup;float t=smooth(p.progress/s.windup);p.angle=start+(s.startAngle-start)*t;}
 else if(p.progress<s.windup+s.swing){p.phase=Phase::Swing;p.damaging=true;float t=(p.progress-s.windup)/s.swing;t=1-std::pow(1-t,3.f);p.angle=s.startAngle+(s.endAngle-s.startAngle)*t;}
 else {p.phase=Phase::Recovery;float t=smooth((p.progress-s.windup-s.swing)/s.recovery);p.angle=s.endAngle+(end-s.endAngle)*t;}
 p.armAngle=p.angle*.6f+.3f;p.backArmAngle=2.1f;
 p.bodyLean=(p.damaging?.13f:-.06f)*p.direction*p.gravity;p.bodyDrop=(heavy?2.f:1.f)*p.gravity;
 p.handOffset={std::cos(p.armAngle)*12*p.direction,std::sin(p.armAngle)*12*p.gravity};
 p.grip=anchor+p.handOffset;p.tip=p.grip+Vec2{std::cos(p.angle)*blade*p.direction,std::sin(p.angle)*blade*p.gravity};return p;
}
}

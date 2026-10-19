#pragma once
#include <algorithm>
#include <cmath>
#include "combat/attack_types.hpp"
namespace mr {
// Shared sprite playback parameters, independent of the graphics backend.
struct SlashSheetPose {
  bool visible{},mirrored{}; int frame{}; float opacity{},radius{},rotation{},glow{};
};
inline SlashSheetPose slashSheet(float progress,bool heavy,float bladeAngle=0) {
  SlashSheetPose p{};
  if(!std::isfinite(progress) || progress<.22f || progress>.78f) return p;
  float u=std::clamp((progress-.22f)/.56f,0.f,1.f);
  p.visible=true;p.frame=std::min(5,int(u*6));
  p.opacity=std::clamp(std::min(u*12.f,(1.f-u)*5.f),0.f,1.f);
  p.radius=heavy?62.f:54.f;
  // Author-frame centroids, measured about the shared canvas pivot.
  constexpr float centers[]={-2.816399f,2.256604f,1.130962f,-.011137f,-.823844f,-1.398785f};
  p.rotation=bladeAngle-.60f-centers[p.frame];
  p.glow=p.opacity*(heavy?.45f:.28f);
  return p;
}
inline SlashSheetPose slashForAttack(const AttackPose &attack,const SwingProfile &profile) {
  if(!attack.active || attack.charging || attack.progress<profile.windup) return {};
  float q=(attack.progress-profile.windup)/(profile.swing+.18f);
  if(q>1) return {};
  auto p=slashSheet(.22f+q*.56f,attack.charged,attack.angle);
  p.mirrored=profile.endAngle<profile.startAngle;
  if(p.mirrored) p.rotation=2*attack.angle-p.rotation;
  p.radius=attack.bladeLength*(attack.charged?1.2f:1.1f);
  return p;
}
}

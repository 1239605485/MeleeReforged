#include "swing_renderer.hpp"
#include <algorithm>
namespace mr {
DrawCommands SwingRenderer::build(const AttackPose &p,const TrailBuffer &t,Quality q) {
  DrawCommands out;
  if(!p.active) return out;
  auto color=[&](int band,unsigned char alpha) -> Color {
    if(p.charged || p.charging) {
      if(band==0) return {255,140,30,alpha};
      if(band==1) return {255,211,94,alpha};
      return {255,248,210,alpha};
    }
    if(band==0) return {25,125,220,alpha};
    if(band==1) return {65,220,255,alpha};
    return {215,255,255,alpha};
  };
  if(p.charging) {
    // Compact, bounded charge glints at the hilt and blade, no screen rays.
    float pulse=.5f+.5f*std::sin(float(p.tick)*.25f);
    Vec2 v=p.tip-p.grip;
    auto at=p.grip+v*.82f;
    float size=1.5f+p.chargeLevel*2.f;
    auto a=static_cast<unsigned char>((90+100*pulse)*p.chargeLevel);
    out.add({at-Vec2{size,0},at+Vec2{size,0},1,color(2,a),3});
    out.add({at-Vec2{0,size},at+Vec2{0,size},1,color(2,a),3});
    for(int i=0;i<8;++i) {
      float phase=std::fmod(float(p.tick)*.025f+i*.125f,1.f);
      float theta=i*2.39996f;
      Vec2 v0{std::cos(theta),std::sin(theta)};
      Vec2 pos=at+v0*(8.f+16.f*(1-phase));
      out.add({pos,pos-v0*2.f,1,color(1,static_cast<unsigned char>(phase*p.chargeLevel*150)),1});
    }
    return out;
  }
  auto max=budget(q).points;
  std::size_t begin=t.size()>max?t.size()-max:0;
  for(std::size_t i=begin+1;i<t.size();++i) {
    const auto &v=t[i]; const auto &old=t[i-1];
    if(v.pose.attackId!=p.attackId || old.pose.attackId!=p.attackId) continue;
    float life=p.charged?.16f:.10f;
    float fade=std::clamp(1-(t.time()-v.time)/life,0.f,1.f);
    // Connect concentric blade samples into a tapered crescent instead of
    // drawing radial spokes from each historical hilt.
    for(int band=0;band<3;++band) {
      float radius=.68f+.15f*band;
      Vec2 a=old.pose.grip+(old.pose.tip-old.pose.grip)*radius;
      Vec2 b=v.pose.grip+(v.pose.tip-v.pose.grip)*radius;
      if(length(b-a)>p.bladeLength*1.5f) continue;
      auto alpha=static_cast<unsigned char>(fade*(band==2?190:100));
      float width=(band==2?1.4f:(p.charged?6.f:4.f))*fade;
      out.add({a,b,width,color(band,alpha),band==2?3:1});
    }
  }
  if(p.damaging) {
    Vec2 v=p.tip-p.grip;
    out.add({p.grip+v*.62f,p.tip,p.charged?2.f:1.f,color(2,190),3});
    if(!p.charged && p.combo==2) {
      Vec2 n{-v.y/p.bladeLength,v.x/p.bladeLength};
      out.add({p.grip-v*.2f,p.tip,7,color(0,55),1});
      out.add({p.grip,p.tip,3,color(1,120),1});
      out.add({p.tip-v*.22f+n*4,p.tip,1,color(2,200),3});
      out.add({p.tip-v*.22f-n*4,p.tip,1,color(2,200),3});
    }
  }
  return out;
}
}

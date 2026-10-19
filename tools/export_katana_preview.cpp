#include "combat/trajectory.hpp"
#include "render/katana_fan.hpp"
#include "render/weapon_sprite.hpp"
#include <cstdio>
int main(){
 for(int frame=0;frame<108;++frame)for(int heavy=0;heavy<2;++heavy)for(int d:{1,-1}){
  float t=frame/60.f,charge=0,u=0;bool charging=false;
  if(heavy){if(t<.65f){charging=true;charge=std::min(1.f,t/.55f);}else u=std::min(1.f,(t-.65f)/.56f);}
  else u=std::clamp((t-.15f)/.4f,0.f,1.f);
  auto p=mr::evaluatePose(mr::katana(),0,charging?mr::katana().heavy[0].windup*charge:u,{},d,1,34,heavy);
  p.charging=charging;p.chargeLevel=charge;p.tick=frame;
  auto f=mr::katanaFan(p,.4f);auto s=mr::weaponSprite(2273,48,54,p.grip,p.tip,d,1);
  std::printf("%d,%d,%d,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%d,%d,%d,%.6f,%.6f,%.6f,%d,%.6f,%.6f,%.6f,%.6f,%.6f,%d\n",frame,heavy,d,p.grip.x,p.grip.y,p.tip.x,p.tip.y,p.bodyLean,p.bodyDrop,charging,f.visible,f.frame,f.rotation,f.scale,f.opacity,f.row,s.rotation,s.scale,s.origin.x,s.origin.y,charge,s.effects);
 }
}

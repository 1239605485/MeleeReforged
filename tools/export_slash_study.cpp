#include "combat/trajectory.hpp"
#include "render/slash_sheet.hpp"
#include "weapons/weapon_registry.hpp"
#include <cstdio>
int main() {
 using namespace mr;
 std::puts("[");
 for(int i=0;i<120;++i) {
   float u=float(i)/119;
   auto p=evaluatePose(*findWeapon(24),0,u,{0,0},1,1,42);
   auto fx=slashSheet(u,false,p.angle);
   std::printf("{\"u\":%.6f,\"angle\":%.6f,\"tip\":[%.6f,%.6f],\"phase\":%d,\"frame\":%d,\"visible\":%s,\"opacity\":%.6f,\"radius\":%.6f,\"rotation\":%.6f,\"glow\":%.6f}%s\n",u,p.angle,p.tip.x,p.tip.y,int(p.phase),fx.frame,fx.visible?"true":"false",fx.opacity,fx.radius,fx.rotation,fx.glow,i==119?"":",");
 }
 std::puts("]");
}

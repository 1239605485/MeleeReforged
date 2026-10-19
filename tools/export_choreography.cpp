#include "combat/trajectory.hpp"
#include "render/slash_sheet.hpp"
#include "weapons/weapon_registry.hpp"
#include <cstdio>
int main() {
 using namespace mr;
 const auto &weapon=*findWeapon(24);
 std::puts("[");
 for(int heavy=0;heavy<2;++heavy) for(int move=0;move<3;++move) for(int frame=0;frame<=60;++frame) {
  auto p=evaluatePose(weapon,move,frame/60.f,{0,0},1,1,31,heavy);
  auto fx=slashForAttack(p,heavy?weapon.heavy[move]:weapon.swings[move]);
  std::printf("{\"heavy\":%d,\"move\":%d,\"u\":%.6f,\"phase\":%d,\"blade\":%.6f,\"arm\":%.6f,\"back\":%.6f,\"lean\":%.6f,\"drop\":%.6f,\"shift\":%.6f,\"leg\":%.6f,\"legShift\":%.6f,\"grip\":[%.6f,%.6f],\"tip\":[%.6f,%.6f],\"fx\":%s,\"frame\":%d,\"opacity\":%.6f,\"rotation\":%.6f,\"radius\":%.6f,\"mirrored\":%s}%s\n",
    heavy,move,p.progress,int(p.phase),p.angle,p.armAngle,p.backArmAngle,p.bodyLean,p.bodyDrop,p.bodyShift,p.legLean,p.legShift,p.grip.x,p.grip.y,p.tip.x,p.tip.y,fx.visible?"true":"false",fx.frame,fx.opacity,fx.rotation,fx.radius,fx.mirrored?"true":"false",
    heavy==1 && move==2 && frame==60?"":",");
 }
 std::puts("]");
}

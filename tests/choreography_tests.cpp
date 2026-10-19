#include "combat/attack_button.hpp"
#include "combat/step_impulse.hpp"
#include "combat/attack_controller.hpp"
#include "collision/damage_sweep.hpp"
#include "render/swing_renderer.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#define CHECK(x) do { if(!(x)) {std::fprintf(stderr,"line %d: %s\n",__LINE__,#x);std::exit(1);} } while(0)
using namespace mr;
int main() {
  AttackButton key;
  CHECK(!key.advance(true,0).normal);
  CHECK(!key.advance(true,.1f).heavy);
  auto tap=key.advance(false,0); CHECK(tap.normal && !tap.heavy);
  CHECK(!key.advance(false,.1f).normal);
  key.advance(true,0);
  for(int i=0;i<40;++i) CHECK(!key.advance(true,1.f/60).heavy);
  auto release=key.advance(false,0); CHECK(release.heavy && !release.normal);
  CHECK(!key.advance(false,.1f).heavy);
  key.advance(true,0); key.advance(true,.4f); key.reset();
  CHECK(!key.advance(false,0).heavy);
  AttackInput in{}; in.itemId=24;in.alive=in.inWorld=in.singlePlayer=true;
  in.cycleSeconds=.3f; in.bladeLength=31;in.charging=true; in.chargeLevel=1;
  AttackController c; auto p=c.advance(in); CHECK(p.active && p.charging && !p.damaging);
  CHECK(collectDamageSweep({},p).count==0);
  auto chargePose=p;
  in.tick++;in.charging=false;in.chargedAttack=in.attacking=true;
  p=c.advance(in);CHECK(p.active && p.charged && !p.charging && p.combo==0);
  CHECK(collectDamageSweep(chargePose,p).count==0);
  in.attacking=in.chargedAttack=false; in.dtSeconds=.45f;
  in.attacking=false;in.tick++;p=c.advance(in); CHECK(!p.active);
  // A released heavy must not force the remaining two attacks.
  c.reset(); in.dtSeconds=0;in.attacking=true;in.chargedAttack=true;in.tick++;
  p=c.advance(in); CHECK(p.combo==0);
  in.chargedAttack=false;in.dtSeconds=.45f;in.tick++;
  p=c.advance(in); CHECK(p.active && p.charged && p.combo==1);
  in.tick++;p=c.advance(in); CHECK(p.charged && p.combo==2);
  in.attacking=false;in.tick++;p=c.advance(in); CHECK(!p.active);
  // A full charged release queued during a normal attack starts C, not another A.
  c.reset();in.dtSeconds=0;in.chargedAttack=false;in.attacking=true;in.tick++;
  CHECK(!c.advance(in).charged);
  in.dtSeconds=.3f;in.chargedAttack=true;in.tick++;
  p=c.advance(in);CHECK(p.active && p.charged && p.combo==0);
  // A confirmed hit briefly holds this attack, without pausing simulation/input.
  c.reset();in.dtSeconds=0;in.attacking=true;in.chargedAttack=false;in.tick++;
  c.advance(in);in.attacking=false;in.dtSeconds=.12f;in.tick++;p=c.advance(in);
  CHECK(p.damaging);float heldProgress=p.progress;
  in.hitConfirmed=true;in.dtSeconds=1.f/60;in.tick++;p=c.advance(in);
  CHECK(std::abs(p.progress-heldProgress)<.001f);
  in.hitConfirmed=false;in.tick++;p=c.advance(in);
  CHECK(std::abs(p.progress-heldProgress)<.001f);
  in.tick++;p=c.advance(in);CHECK(p.progress>heldProgress);
  // Left/right and gravity transforms preserve the same reach for all moves.
  for(bool heavy:{false,true}) for(int move=0;move<3;++move) {
    auto a=evaluatePose(*findWeapon(24),move,.4f,{0,0},1,1,31,heavy);
    auto b=evaluatePose(*findWeapon(24),move,.4f,{0,0},-1,-1,31,heavy);
    CHECK(std::abs(a.tip.x+b.tip.x)<.001f && std::abs(a.tip.y+b.tip.y)<.001f);
    CHECK(length(a.tip-a.grip)<=31.01f);
  }
  auto thrustA=evaluatePose(*findWeapon(24),2,.35f,{0,0},1,1,31);
  auto thrustB=evaluatePose(*findWeapon(24),2,.45f,{0,0},1,1,31);
  thrustA.attackId=thrustB.attackId=12;
  auto thrustSweep=collectDamageSweep(thrustA,thrustB);
  CHECK(thrustSweep.count>1);
  CHECK(length(thrustSweep.segments[thrustSweep.count-1].tip-thrustB.tip)<.001f);
  for(bool heavy:{false,true}) for(int move=0;move<2;++move) {
    auto end=evaluatePose(*findWeapon(24),move,1,{0,0},1,1,31,heavy);
    auto next=evaluatePose(*findWeapon(24),move+1,0,{0,0},1,1,31,heavy);
    CHECK(std::abs(end.angle-next.angle)<.001f);
    CHECK(length(end.handOffset-next.handOffset)<.001f);
  }
  auto thrustStart=evaluatePose(*findWeapon(24),2,.29f,{0,0},1,1,31);
  auto thrustEnd=evaluatePose(*findWeapon(24),2,.55f,{0,0},1,1,31);
  CHECK(std::abs(thrustStart.angle-thrustEnd.angle)<.15f);
  CHECK(thrustEnd.grip.x-thrustStart.grip.x>10.f);
  auto stride=evaluatePose(*findWeapon(24),0,.4f,{0,0},1,1,31);
  CHECK(stepVelocity(stride,{0,0},true).x>0);
  CHECK(stepVelocity(stride,{0,-3},true).x==0);
  CHECK(stepVelocity(stride,{5,0},true).x==5);
  CHECK(stepVelocity(stride,{0,0},false).x==0);
  CHECK(stepVelocity(stride,{-2,0},true).x==-2);
  stride.direction=-1;CHECK(stepVelocity(stride,{0,0},true).x<0);
  stride.damaging=false;CHECK(stepVelocity(stride,{0,0},true).x==0);
  auto native=toNativeColor({20,100,220,128});
  unsigned char bytes[4];std::memcpy(bytes,&native,4);
  CHECK(bytes[0]==128 && bytes[1]==110 && bytes[2]==50 && bytes[3]==10);
  auto p0=evaluatePose(*findWeapon(24),0,.32f,{0,0},1,1,31);
  auto p1=evaluatePose(*findWeapon(24),0,.36f,{0,0},1,1,31);
  p0.attackId=p1.attackId=7;TrailBuffer t;t.push(1,p0);t.push(2,p1);
  auto cmd=SwingRenderer::build(p1,t,Quality::Medium);
  CHECK(cmd.count>3 && cmd.count<=320);
  for(std::size_t i=0;i<cmd.count;++i) {
    CHECK(length(cmd.data[i].to-cmd.data[i].from)<63);
    CHECK(cmd.data[i].width<=10);
  }
  std::puts("choreography checks passed");
}

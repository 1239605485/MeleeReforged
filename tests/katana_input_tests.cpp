#include "combat/attack_button.hpp"
#include "combat/attack_controller.hpp"
#include <cassert>
int main(){
 mr::AttackButton input;mr::AttackController c;mr::AttackPose pose{};
 mr::AttackInput in{};in.itemId=2273;in.alive=in.inWorld=in.singlePlayer=true;
 in.cycleSeconds=.4f;in.dtSeconds=1.f/60;in.bladeLength=34;
 auto step=[&](bool down){auto x=input.advance(down,in.dtSeconds);in.tick++;
  in.charging=x.holding && (!pose.active || pose.charging);in.chargeLevel=x.level;
  in.attacking=x.normal || x.heavy;in.chargedAttack=x.heavy;pose=c.advance(in);return x;};
 step(true);step(true);auto tap=step(false);assert(tap.normal && !tap.heavy && pose.active && !pose.charged);
 for(int i=0;i<60;++i){step(false);assert(pose.combo==0);}assert(!pose.active);
 for(int i=0;i<40;++i){step(true);assert(pose.charging && !pose.damaging);}
 auto release=step(false);assert(release.heavy && pose.active && pose.charged);
 auto id=pose.attackId;bool damage=false;
 for(int i=0;i<60;++i){auto x=step(false);assert(!x.heavy);if(pose.active){assert(pose.attackId==id && pose.combo==0);damage|=pose.damaging;}}
 assert(damage && !pose.active);
}

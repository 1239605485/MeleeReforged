#include "combat/katana_input.hpp"
#include <cassert>
#include "combat/attack_controller.hpp"
int main(){
 mr::KatanaInput input;
 auto x=input.advance(true,false,0,true);assert(x.normal && !x.holding && !x.heavy);
 for(int i=0;i<120;++i){x=input.advance(true,false,1.f/60,true);assert(x.normal && !x.holding && !x.heavy);}
 x=input.advance(false,false,1.f/60,true);assert(!x.heavy && !x.normal);
 x=input.advance(true,true,0,true);assert(x.holding && !x.normal);
 for(int i=0;i<40;++i)x=input.advance(true,false,1.f/60,true);
 assert(x.holding && x.level==1);
 x=input.advance(false,false,1.f/60,true);assert(x.heavy && !x.normal);
 assert(!input.advance(false,false,.1f,true).heavy);
 input.advance(true,true,0,true);input.advance(true,true,.1f,true);
 x=input.advance(false,false,.01f,true);assert(!x.heavy && !x.normal);
 input.advance(true,false,0,true);x=input.advance(true,true,1,true);assert(x.normal && !x.holding);
 input.reset();input.advance(true,true,0,false);x=input.advance(true,true,1,false);assert(x.level==0);
 input.reset();assert(!input.advance(false,false,.1f,true).heavy);
 // Replay touch intents through the actual controller, rather than testing
 // recognizer flags alone. A long normal hold must visit all three cuts.
 mr::AttackController controller;mr::AttackInput in{};
 in.itemId=2273;in.alive=in.inWorld=in.singlePlayer=true;in.cycleSeconds=.3f;
 in.bladeLength=34;in.dtSeconds=1.f/60;
 bool combos[3]{};mr::AttackPose pose{};
 input.reset();
 auto step=[&](bool attack,bool down){
  auto intent=input.advance(attack,down,in.dtSeconds,!pose.active || pose.charging);
  in.tick++;in.charging=intent.holding && (!pose.active || pose.charging);
  in.chargeLevel=intent.level;in.attacking=intent.normal || intent.heavy;in.chargedAttack=intent.heavy;
  pose=controller.advance(in);return pose;
 };
 for(int i=0;i<120;++i){auto p=step(true,false);assert(p.active && !p.charging && !p.charged);combos[p.combo]=true;}
 assert(combos[0] && combos[1] && combos[2]);
 for(int i=0;i<45;++i)step(false,false);
 assert(!pose.active);
 for(int i=0;i<40;++i){step(true,true);assert(pose.charging && !pose.damaging);}
 step(false,false);assert(pose.active && pose.charged && !pose.charging);
 auto attackId=pose.attackId;
 bool hadDamage=false;
 for(int i=0;i<45;++i){step(false,false);if(pose.active){assert(pose.attackId==attackId && pose.charged);hadDamage|=pose.damaging;}}
 assert(hadDamage && !pose.active);
 step(true,false);assert(pose.combo==0 && !pose.charged);

}

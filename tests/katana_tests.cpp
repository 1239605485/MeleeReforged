#include "weapons/weapon_registry.hpp"
#include "combat/trajectory.hpp"
#include "combat/attack_controller.hpp"
#include "collision/damage_sweep.hpp"
#include <cassert>
#include <cmath>
int main(){
 auto w=mr::findWeapon(2273);assert(w && "Katana must have an independent profile");
 auto a=mr::evaluatePose(*w,0,.3f,{},1,1,34,false);
 auto b=mr::evaluatePose(*w,0,.3f,{},-1,-1,34,false);
 assert(std::abs(a.tip.x+b.tip.x)<.001f && std::abs(a.tip.y+b.tip.y)<.001f);
 auto h=mr::evaluatePose(*w,0,.23f,{},1,1,34,true);
 assert(h.damaging);
 auto prior=mr::evaluatePose(*w,0,.1f,{},1,1,34,true);
 prior.attackId=h.attackId=1;
 assert(mr::collectDamageSweep(prior,h).count>1);
 mr::AttackController c;mr::AttackInput in{};
 in.itemId=2273;in.alive=in.inWorld=in.singlePlayer=true;in.attacking=in.chargedAttack=true;
 in.cycleSeconds=.3f;in.bladeLength=34;
 c.advance(in);bool ended=false;
 for(int i=1;i<90;++i){in.tick=i;in.dtSeconds=1.f/60;auto p=c.advance(in);if(!p.active){ended=true;break;}assert(p.combo==0);}
 assert(ended && "Katana heavy must finish after one cut, without wooden followups");
 in.tick+=1;in.chargedAttack=false;in.dtSeconds=1.f/60;
 assert(c.advance(in).combo==0 && "Normal combo must restart after the independent heavy");
 assert(mr::findWeapon(24) && !mr::findWeapon(1));
}

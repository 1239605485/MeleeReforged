#include "carry/registry.hpp"
#include "carry/state.hpp"
#include "carry/pose.hpp"
#include <cstdio>
#include <cstdlib>
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"line %d: %s\n",__LINE__,#x);std::exit(1);}}while(0)
using namespace mr;
int main(){
 CarryTraits t{};t.itemId=1;t.damage=5;t.pick=35;
 CHECK(carryProfile(t,32,32).kind==CarryKind::Pickaxe);
 t.axe=10;CHECK(carryProfile(t,32,32).kind==CarryKind::MultiTool);
 t.pick=0;t.axe=0;t.hammer=20;CHECK(carryProfile(t,64,64).back);
 t={};t.itemId=2;t.ranged=true;t.damage=10;t.bullet=true;t.useStyle=5;
 CHECK(carryProfile(t,28,16).kind==CarryKind::Handgun);
 CHECK(carryProfile(t,58,22).kind==CarryKind::LongGun);
 t.arrow=true;t.bullet=false;CHECK(carryProfile(t,20,48).kind==CarryKind::Bow);
 t={};t.itemId=5478;t.damage=10;t.summon=true;t.noUseGraphic=true;
 CHECK(carryProfile(t,18,80).kind==CarryKind::Whip);
 t.itemId=4672;CHECK(carryProfile(t,18,80).kind==CarryKind::Whip);
 t={};t.itemId=3;t.consumable=true;t.damage=10;t.ranged=true;
 CHECK(!carryProfile(t,20,20).valid());
 t={};t.itemId=4;t.magic=true;t.damage=10;
 CHECK(carryProfile(t,44,44).kind==CarryKind::Book);
 t.staff=true;CHECK(carryProfile(t,44,44).kind==CarryKind::Staff);
 CarryPresentationState s;
 s.observe(2,true,false,1,0);CHECK(s.visible() && s.amount()==1);
 s.observe(2,true,true,2,.016f);CHECK(!s.visible());
 s.observe(2,true,false,3,.1f);CHECK(s.visible() && s.amount()==0);
 s.observe(2,true,false,4,.3f);CHECK(s.amount()>.7f);
 s.observe(2,true,false,4,.3f);CHECK(s.amount()<1);
 s.observe(3,true,false,5,.016f);CHECK(s.amount()==1 && !s.hasUsePose);
 auto profile=carryProfile(t,44,44);
 auto right=carriedPose(profile,{0,0},20,42,1,1,0,false);
 auto left=carriedPose(profile,{0,0},20,42,-1,-1,0,false);
 CHECK(std::abs(right.position.x+left.position.x-20)<.001f);
 CHECK(std::abs(right.position.y+left.position.y-42)<.001f);
 CHECK(!carriedPose(profile,{0,0},20,42,1,1,0,true).back);
 std::puts("generic carry checks passed");
}

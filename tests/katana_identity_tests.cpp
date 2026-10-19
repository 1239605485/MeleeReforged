#include "combat/attack_controller.hpp"
#include "weapons/weapon_registry.hpp"
#include "render/weapon_sprite.hpp"
#include <cassert>
#include <cmath>
int main(){
 assert(mr::katana().normalStages==1 && mr::woodenSword().normalStages==3);
 mr::AttackController c;mr::AttackInput in{};in.itemId=2273;in.alive=in.inWorld=in.singlePlayer=true;in.attacking=true;in.cycleSeconds=.4f;in.dtSeconds=1.f/60;in.bladeLength=34;
 for(int i=0;i<150;++i){in.tick=i;auto p=c.advance(in);assert(p.combo==0);}
 for(int d:{-1,1})for(float g:{-1.f,1.f}){
  mr::Vec2 grip{20,30},tip=grip+mr::Vec2{.8f*34*d,.6f*34*g};
  auto sprite=mr::weaponSprite(2273,48,54,grip,tip,d,g);
  mr::Vec2 axis=sprite.sourceTip-sprite.origin;
  auto mapped=mr::Vec2{std::cos(sprite.rotation)*axis.x-std::sin(sprite.rotation)*axis.y,std::sin(sprite.rotation)*axis.x+std::cos(sprite.rotation)*axis.y}*sprite.scale+grip;
  assert(mr::length(mapped-tip)<.001f);
  assert(sprite.effects==((d*g<0)?2:0));
 }
}

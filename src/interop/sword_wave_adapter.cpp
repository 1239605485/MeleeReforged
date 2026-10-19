#include "game_runtime.hpp"
#include "weapons/wooden_sword_rules.hpp"
#include "weapons/weapon_registry.hpp"
#include <algorithm>
namespace mr {
void GameRuntime::launchWave(Handle player,PlayerState &state,Handle item){
 const auto &pose=state.current;
 if(pose.itemId!=24 || !api_.wavesReady || !pose.active || pose.charging || !pose.charged || pose.combo!=0 ||
    state.launchedAttack==pose.attackId)return;
 const auto &profile=findWeapon(24)->heavy[0];
 if(pose.progress<profile.windup+profile.swing*.45f)return;
 state.launchedAttack=pose.attackId;
 auto center=api_.read<Vec2>(Field::Position,player)+
   Vec2{api_.read<int>(Field::Width,player)*.5f,api_.read<int>(Field::Height,player)*.5f};
 auto origin=pose.grip+Vec2{pose.direction*pose.bladeLength*.65f,0};
 origin.y=center.y; // Keep the horizontal crescent clear of the floor during a low grip pose.
 auto corridor=traceWavePath(center,origin,[&](Vec2 point){
   return invokeStatic<bool>(api_.solidCollision,point-Vec2{4,18},8,36,false);
 });
 if(corridor.blocked)return;
 for(auto &wave:state.waves)if(!wave.active){
  int damage=scaleDamage(invoke<int>(api_.weaponDamage,player,item),woodenWavePower.damage);
  float knock=invoke<float>(api_.weaponKnockback,player,item,api_.read<float>(Field::ItemKnockback,item));
  if(damage>0 && std::isfinite(knock))
   wave.launch(origin,pose.direction,
               pose.gravity,damage,knock*woodenWavePower.knockback);
  break;
 }
}
void GameRuntime::updateWaves(Handle player,PlayerState &state){
 if(!api_.wavesReady || api_.read<bool>(Field::Paused))return;
 auto tick=invokeStatic<unsigned>(api_.tick);
 if(state.lastWaveTick==tick)return;
 float dt=state.lastWaveTick!=~std::uint64_t(0) && tick>state.lastWaveTick?
   std::min(.1f,float(tick-state.lastWaveTick)/60.f):0;
 state.lastWaveTick=tick;
 auto npcs=api_.read<Handle>(Field::Npcs);
 if(!npcs)return;
 for(auto &wave:state.waves){
  if(!wave.active)continue;
  auto motion=wave.step(dt);
  float travel=length(motion.to-motion.from);
  if(travel<=0)continue;
  auto path=traceWavePath(motion.from,motion.to,[&](Vec2 point){
   return invokeStatic<bool>(api_.solidCollision,point-Vec2{4,18},8,36,false);
  });
  Vec2 clear=path.end;
  if(path.blocked){wave.active=false;wave.position=clear;}
  Rect sweep{std::min(motion.from.x,clear.x)-4,clear.y-18,
             std::abs(clear.x-motion.from.x)+8,36};
  if(length(clear-motion.from)<.001f)continue;
  for(int slot=0;slot<200 && static_cast<std::size_t>(slot)<patchlib_array_length(npcs);++slot){
   Handle npc{};
   if(!wave.canHit(slot,npcGenerations_[slot]) || !patchlib_array_at(npcs,slot,&npc) || !npc ||
      !api_.read<bool>(Field::NpcActive,npc) || api_.read<int>(Field::NpcLife,npc)<=0 ||
      api_.read<bool>(Field::NpcFriendly,npc) || api_.read<bool>(Field::NpcImmortal,npc) ||
      api_.read<bool>(Field::NpcInvulnerable,npc))continue;
   auto pos=api_.read<Vec2>(Field::Position,npc);
   float width=float(api_.read<int>(Field::Width,npc)),height=float(api_.read<int>(Field::Height,npc));
   if(pos.x>sweep.x+sweep.w || pos.x+width<sweep.x || pos.y>sweep.y+sweep.h || pos.y+height<sweep.y)continue;
   wave.markHit(slot,npcGenerations_[slot]);
   int before=api_.read<int>(Field::NpcLife,npc);
   auto source=invokeStatic<NativeHitSource>(api_.hitSource,24);
   invoke<void>(api_.applyDamage,player,npc,wave.damage,wave.knockback,wave.direction,false,source);
   if(api_.read<int>(Field::NpcLife,npc)<before)
    state.impacts[state.nextImpact++%state.impacts.size()]={pos+Vec2{width*.5f,height*.5f},tick,true};
  }
 }
}
}

#include "combat/trajectory.hpp"
#include "combat/step_impulse.hpp"
#include "weapons/weapon_registry.hpp"
#include "game_runtime.hpp"
#include <algorithm>
namespace mr {
void GameRuntime::restorePose(Handle p) {
  if(!ready_.load()) return;
  auto s=state(p);
  if(!s || !s->bodyApplied) return;
  api_.write(Field::BodyRotation,p,s->baseBodyRotation);
  api_.write(Field::BodyPosition,p,s->baseBodyPosition);
  api_.write(Field::LegRotation,p,s->baseLegRotation);
  api_.write(Field::LegPosition,p,s->baseLegPosition);
  api_.write(Field::FrontArm,p,s->baseFrontArm);
  api_.write(Field::BackArm,p,s->baseBackArm);
  s->bodyApplied=false;
}
bool GameRuntime::validScene() {
  return ready_.load() && !invokeStatic<bool>(api_.menu) &&
         api_.read<int>(Field::NetMode) == 0;
}
void GameRuntime::frame() {
  if (!ready_.load())
    return;
  bool inWorld =
      !invokeStatic<bool>(api_.menu) && api_.read<int>(Field::NetMode) == 0;
  if(!inWorld || api_.read<bool>(Field::Paused)){
    restoreWeaponApiPoses();
  }
  if (inWorld != wasInWorld_) {
    states_.clear();
    for (auto &g : npcGenerations_)
      ++g;
    renderFailed_ = false;
    wasInWorld_ = inWorld;
  }
}
void GameRuntime::resetNpc(int id) {
  if (id >= 0 && id < 200)
    ++npcGenerations_[id];
}
PlayerState *GameRuntime::state(Handle p) {
  if (!p)
    return nullptr;
  auto slot = api_.read<int>(Field::PlayerId, p);
  auto s = states_.get(slot);
  if (!s)
    return nullptr;
  auto world =
      reinterpret_cast<std::uintptr_t>(api_.read<Handle>(Field::World));
  auto id = reinterpret_cast<std::uintptr_t>(p);
  if (s->identity != id || s->world != world) {
    s->clear();
    s->identity = id;
    s->world = world;
    ++s->generation;
  }
  return s;
}
void GameRuntime::update(Handle p) {
  if (!ready_.load())
    return;
  if (!validScene()) {
    states_.clear();
    return;
  }
  auto s = state(p);
  if (!s)
    return;
  auto item = invoke<Handle>(api_.heldItem, p);
  if (!item || api_.read<bool>(Field::Dead, p) || !api_.read<bool>(Field::Active, p)) {
    cancelWeaponApiPlayer(p);
    s->clear();
    return;
  }
  if(updateWeaponApi(p,*s,item))return;
  if(!findWeapon(api_.read<int>(Field::ItemType,item))){
    if(s->seen)s->clearEquipment();
    updateGenericCarry(p,*s,item);
    return;
  }
  if(s->presentation.itemId)s->clearEquipment();
  auto tick = invokeStatic<unsigned>(api_.tick);
  // A no-use tick still completes recovery, ages trails and advances combo idle
  // timeout.
  // Reapply after vanilla Update may have rewritten the body/arm pose.
  // action() advances input/controller at most once for this simulation tick.
  (void)tick;
  action(p);
  updateWaves(p,*s);
}
void GameRuntime::action(Handle p) {
  if (!validScene() || api_.read<bool>(Field::Paused) || !resourcesReady())
    return;
  auto s = state(p);
  if (!s)
    return;
  auto item = invoke<Handle>(api_.heldItem, p);
  if (!item || api_.read<bool>(Field::Dead, p) || !api_.read<bool>(Field::Active, p)) {
    s->clear();
    return;
  }
  auto itemId=api_.read<int>(Field::ItemType,item);
  auto weapon=findWeapon(itemId);if(!weapon)return;
  if(s->combatItemId && s->combatItemId!=itemId)s->clearEquipment();
  if(s->presentation.itemId)s->clearEquipment();
  s->combatItemId=itemId;
  auto now = invokeStatic<unsigned>(api_.tick);
  if (s->seen && now == s->lastTick) {
    if (s->current.active || s->carry.ready())
      apply(p, *s);
    return;
  }
  float dt = s->seen && now > s->lastTick
                 ? std::min(.1f, float(now - s->lastTick) / 60.f)
                 : 0.f;
  auto pos = api_.read<Vec2>(Field::Position, p);
  float scale = api_.read<float>(Field::ItemScale, item);
  AttackInput in{};
  in.tick = now;
  in.playerGeneration = s->generation;
  in.worldGeneration = s->world;
  in.itemId = itemId;
  in.direction = api_.read<int>(Field::Direction, p);
  in.gravity = api_.read<float>(Field::Gravity, p);
  int animationMax=api_.read<int>(Field::AnimationMax,p);
  if(animationMax>2 && !s->current.active)
    s->cycleSeconds=std::clamp(animationMax/60.f,.16f,.65f);
  in.cycleSeconds=s->cycleSeconds;
  in.dtSeconds = dt;
  in.grip = pos + Vec2{api_.read<int>(Field::Width, p) * .5f,
                       api_.read<int>(Field::Height, p) * .4f};
  in.bladeLength = weapon->bladeLength * scale;
  bool canStart=!s->current.active || s->current.charging;
  auto intent=s->button.advance(api_.read<bool>(Field::ControlUseItem,p),dt);
  // Touch: tap release cuts once; full hold release performs the heavy.
  s->pendingNormal=s->pendingNormal || intent.normal;
  s->pendingHeavy=s->pendingHeavy || intent.heavy;
  in.charging=intent.holding && canStart;
  in.chargeLevel=intent.level;
  in.attacking=s->pendingNormal || s->pendingHeavy;
  in.chargedAttack=in.attacking && s->pendingHeavy;
  s->baseBodyPosition=api_.read<Vec2>(Field::BodyPosition,p);
  s->baseBodyRotation=api_.read<float>(Field::BodyRotation,p);
  s->baseLegPosition=api_.read<Vec2>(Field::LegPosition,p);
  s->baseLegRotation=api_.read<float>(Field::LegRotation,p);
  s->baseFrontArm=api_.read<std::array<unsigned char,12>>(Field::FrontArm,p);
  s->baseBackArm=api_.read<std::array<unsigned char,12>>(Field::BackArm,p);
  in.alive = true;
  in.inWorld = true;
  in.singlePlayer = true;
  in.hitConfirmed=s->hitConfirmed;s->hitConfirmed=false;
  s->previous = s->current;
  s->current = s->controller.advance(in);
  if(s->current.active && !s->previous.active)
    s->drawingFromWaist=s->carry.amount()>.99f;
  s->carry.advance(s->current.active,dt);
  if(s->current.active && !s->current.charging &&
     (!s->previous.active || s->previous.charging || s->current.attackId!=s->previous.attackId))
    s->pendingNormal=s->pendingHeavy=false;
  s->lastTick = now;
  s->seen = true;
  auto &pose = s->current;
  if (!pose.active) {
    if(s->previous.active) api_.write(Field::Animation,p,0);
    s->trail.clear();
    if(s->carry.ready()) {
      pose=evaluatePose(*weapon,0,0,in.grip,in.direction,in.gravity,in.bladeLength);
      pose.active=pose.damaging=false;
      apply(p,*s);
    }
    return;
  }
  apply(p, *s);
  launchWave(p,*s,item);
  s->trail.push(now, pose);
}
void GameRuntime::apply(Handle p, PlayerState &s) {
  auto &pose = s.current;
  if(pose.itemId==24 && pose.damaging && s.steppedAttack!=pose.attackId) {
    s.steppedAttack=pose.attackId;
    auto mount=api_.read<Handle>(Field::Mount,p);
    bool allowed=(!mount || !invoke<bool>(api_.mountActive,mount)) &&
       !api_.read<bool>(Field::Pulley,p) && !api_.read<bool>(Field::Frozen,p) &&
       !api_.read<bool>(Field::Stoned,p) && api_.read<int>(Field::GrapCount,p)==0;
    if(allowed) {
      auto pos=api_.read<Vec2>(Field::Position,p);
      int width=api_.read<int>(Field::Width,p),height=api_.read<int>(Field::Height,p);
      Vec2 support{pos.x+2,pos.y+(pose.gravity>0?float(height):-2.f)};
      allowed=width>4 && invokeStatic<bool>(api_.solidCollision,support,width-4,2,true);
    }
    auto v=api_.read<Vec2>(Field::Velocity,p);
    api_.write(Field::Velocity,p,stepVelocity(pose,v,allowed));
  }
  api_.write(Field::Direction,p,pose.direction);
  float drawAngle=pose.angle,localArm=pose.armAngle;
  float drawBlend=1;
  if(s.drawingFromWaist && pose.active) {
    const auto &profile=pose.charged?findWeapon(pose.itemId)->heavy[pose.combo]:findWeapon(pose.itemId)->swings[pose.combo];
    drawBlend=pose.charging?std::clamp(pose.chargeLevel*5,0.f,1.f):
       (pose.phase==Phase::Windup?std::clamp(pose.progress/profile.windup,0.f,1.f):1.f);
    drawAngle=2.35f+std::remainder(pose.angle-2.35f,6.283185307f)*drawBlend;
    localArm=1.2f+(pose.armAngle-1.2f)*drawBlend;
    if(drawBlend>=1)s.drawingFromWaist=false;
  }
  float worldAngle=std::atan2(std::sin(drawAngle)*pose.gravity,std::cos(drawAngle)*pose.direction);
  float armAngle=std::atan2(std::sin(localArm)*pose.gravity,
                           std::cos(localArm)*pose.direction)-1.570796327f;
  float backAngle=std::atan2(std::sin(pose.backArmAngle)*pose.gravity,
                            std::cos(pose.backArmAngle)*pose.direction)-1.570796327f;
  // In this Android build Full=0; 3 is ThreeQuarters, not Full.
  invoke<void>(api_.arm,p,true,0,armAngle);
  invoke<void>(api_.backArm,p,true,3,backAngle);
  api_.write(Field::BodyRotation,p,s.baseBodyRotation+pose.bodyLean);
  api_.write(Field::BodyPosition,p,s.baseBodyPosition+Vec2{pose.bodyShift,pose.bodyDrop});
  api_.write(Field::LegRotation,p,s.baseLegRotation+pose.legLean);
  api_.write(Field::LegPosition,p,s.baseLegPosition+Vec2{pose.legShift,0});
  auto grip = invoke<Vec2>(api_.hand,p,0,armAngle);
  if(drawBlend<1) {
    auto waist=waistGrip(api_.read<Vec2>(Field::Position,p),api_.read<int>(Field::Width,p),
       api_.read<int>(Field::Height,p),pose.direction,pose.gravity);
    grip=waist+(grip-waist)*drawBlend;
  }
  pose.tip=grip+Vec2{std::cos(worldAngle)*pose.bladeLength,std::sin(worldAngle)*pose.bladeLength};
  pose.grip = grip;
  // Keep the vanilla ItemCheck path alive for charged/released sequences.
  // Damage callbacks still reject charging, anticipation and recovery.
  if(pose.active && api_.read<int>(Field::Animation,p)<2) api_.write(Field::Animation,p,2);
  s.bodyApplied=true;
  api_.write(Field::Location, p, grip);
  api_.write(Field::Rotation, p, worldAngle);
}
} // namespace mr

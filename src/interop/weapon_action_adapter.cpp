#include "game_runtime.hpp"
#include "draw_bridge.hpp"
#include "item_visuals.hpp"
#include <algorithm>
extern "C" const wa_api_v1 *kernel_weapon_action_api_v1;
namespace mr {
namespace {
Vec2 vector(wa_vec2 v){return {v.x,v.y};}
wa_vec2 vector(Vec2 v){return {v.x,v.y};}
}
void GameRuntime::startWeaponApi(kernel_mod_handle_t *h){
 auto a=kernel_weapon_action_api_v1;
 if(!ready_.load() || !api_.weaponApiReady || !a || a->abi!=WA_ABI_V1 || a->size!=sizeof(*a) || !h)return;
 wa_backend desc{};desc.size=sizeof(desc);desc.abi=WA_ABI_V1;
 desc.capabilities=WA_INPUT|WA_VISUAL_POSE|WA_DRAW;
 if(api_.secondaryInputReady)desc.capabilities|=WA_SECONDARY_INPUT;
 desc.reserved_count=2;desc.reserved_items[0]=24;desc.reserved_items[1]=2273;
 if(a->attach_backend(h,&desc)!=WA_OK){log("Weapon action API backend unavailable; existing melee behavior retained.");return;}
 weaponApiOwner_=h;weaponApiAttached_=true;
 log("Weapon action API v1 backend attached: input, visual pose and item/pixel layers; vanilla damage remains authoritative.");
}
void GameRuntime::restoreWeaponApiPoses(){
 if(!weaponApiAttached_ || !ready_.load())return;
 auto players=api_.read<Handle>(Field::ApiPlayers);
 if(players)for(std::size_t i=0;i<std::min<std::size_t>(256,patchlib_array_length(players));++i){
  auto s=states_.get(int(i));Handle player{};
  if(s && s->apiPoseActive && patchlib_array_at(players,i,&player) && player && s->identity==reinterpret_cast<std::uintptr_t>(player)){
   restorePose(player);s->apiPoseActive=false;
  }
 }
 kernel_weapon_action_api_v1->cancel_all(weaponApiOwner_,WA_SCENE_CHANGED);
}
void GameRuntime::stopWeaponApi(){
 if(!weaponApiAttached_)return;
 restoreWeaponApiPoses();
 kernel_weapon_action_api_v1->detach_backend(weaponApiOwner_);
 weaponApiAttached_=false;weaponApiOwner_=nullptr;
}
void GameRuntime::cancelWeaponApiPlayer(Handle player){
 if(weaponApiAttached_ && player)kernel_weapon_action_api_v1->cancel_player(weaponApiOwner_,api_.read<int>(Field::PlayerId,player),WA_PLAYER_INVALID);
}
bool GameRuntime::updateWeaponApi(Handle player,PlayerState &s,Handle item){
 s.apiPoseActive=false;
 if(!weaponApiAttached_ || api_.read<bool>(Field::Paused))return false;
 int itemId=api_.read<int>(Field::ItemType,item);
 bool wanted=kernel_weapon_action_api_v1->wants_item(weaponApiOwner_,itemId)!=0;
 if(!wanted && s.apiItemId==itemId)return false;
 if(wanted && (s.combatItemId || (s.presentation.itemId && s.presentation.itemId!=itemId)))s.clearEquipment();
 auto now=invokeStatic<unsigned>(api_.tick);wa_input in{};in.size=sizeof(in);
 in.player=api_.read<int>(Field::PlayerId,player);in.item_id=api_.read<int>(Field::ItemType,item);
 in.direction=api_.read<int>(Field::Direction,player);in.gravity=api_.read<float>(Field::Gravity,player);
 in.primary_down=api_.read<bool>(Field::ControlUseItem,player);
 in.secondary_down=api_.secondaryInputReady && api_.read<bool>(Field::ApiSecondaryInput,player);
 in.tick=now;in.world_generation=s.world;in.player_generation=s.generation;
 in.dt=s.apiTick!=~std::uint64_t(0) && now>s.apiTick?std::min(.1f,float(now-s.apiTick)/60.f):0;
 in.position=vector(api_.read<Vec2>(Field::Position,player));in.vanilla_grip=vector(api_.read<Vec2>(Field::Location,player));
 in.vanilla_rotation=api_.read<float>(Field::Rotation,player);in.item_scale=api_.read<float>(Field::ItemScale,item);
 if(wanted){auto visual=itemVisual(api_,itemId);if(visual){in.item_width=visual.frame.w;in.item_height=visual.frame.h;}}
 in.animation=api_.read<int>(Field::Animation,player);int max=api_.read<int>(Field::AnimationMax,player);
 in.animation_progress=max>0?std::clamp(1.f-float(in.animation)/max,0.f,1.f):0;
 wa_pose pose{};int result=kernel_weapon_action_api_v1->update(weaponApiOwner_,&in,&pose);
 s.apiTick=now;s.apiItemId=in.item_id;
 if(result!=WA_OK || !(pose.flags&WA_APPLY_POSE))return false;
 s.baseBodyPosition=api_.read<Vec2>(Field::BodyPosition,player);s.baseBodyRotation=api_.read<float>(Field::BodyRotation,player);
 s.baseLegPosition=api_.read<Vec2>(Field::LegPosition,player);s.baseLegRotation=api_.read<float>(Field::LegRotation,player);
 s.baseFrontArm=api_.read<std::array<unsigned char,12>>(Field::FrontArm,player);s.baseBackArm=api_.read<std::array<unsigned char,12>>(Field::BackArm,player);
 // Pose v1 deliberately does not write Item.rotation/location, animation, time,
 // direction, velocity, damage or projectile fields. Gameplay remains vanilla.
 invoke<void>(api_.arm,player,true,0,pose.front_arm-1.570796327f);
 invoke<void>(api_.backArm,player,true,0,pose.back_arm-1.570796327f);
 api_.write(Field::BodyRotation,player,s.baseBodyRotation+pose.body_lean);
 api_.write(Field::BodyPosition,player,s.baseBodyPosition+vector(pose.body_offset));
 s.apiPose=pose;s.apiPoseActive=true;s.bodyApplied=true;return true;
}
bool GameRuntime::drawWeaponApiBase(Handle ctx,Handle player,PlayerState &s){
 if(!weaponApiAttached_ || !s.apiPoseActive)return false;
 auto item=invoke<Handle>(api_.heldItem,player);
 if(!item || api_.read<int>(Field::ItemType,item)!=s.apiItemId)return false;
 wa_draw_list valid{};
 if(kernel_weapon_action_api_v1->draw(weaponApiOwner_,api_.read<int>(Field::PlayerId,player),s.apiTick,&valid)!=WA_OK)return false;
 auto visual=itemVisual(api_,s.apiItemId);if(!visual)return false;
 auto world=api_.read<Vec2>(Field::Position,player);
 auto screen=invokeStatic<Vec2>(api_.screen)-(api_.read<Vec2>(Field::DrawPosition,ctx)+api_.read<Vec2>(Field::VisualOffset,ctx)-world);
 const auto &p=s.apiPose;auto slot=invoke<Handle>(api_.addDraw,ctx);if(!slot)return false;
 invoke<void>(api_.setDrawRect,slot,visual.texture,vector(p.grip)-screen,visual.frame,toNativeColor({255,255,255,255}),p.sprite_rotation,vector(p.sprite_origin),Vec2{p.scale,p.scale},int(p.sprite_effects),0);
 return true;
}
void GameRuntime::drawWeaponApi(Handle ctx,uint32_t layer){
 if(!weaponApiAttached_ || !validScene() || api_.read<bool>(Field::Paused) || !ctx || api_.read<float>(Field::Shadow,ctx)>0 || api_.read<bool>(Field::DrawHidden,ctx) || api_.read<bool>(Field::DrawHiddenExcept,ctx))return;
 auto player=api_.read<Handle>(Field::DrawPlayer,ctx);if(!player || api_.read<bool>(Field::Dead,player) || !api_.read<bool>(Field::Active,player))return;
 auto s=state(player);if(!s)return;auto item=invoke<Handle>(api_.heldItem,player);
 if(!item || api_.read<int>(Field::ItemType,item)!=s->apiItemId)return;
 wa_draw_list list{};
 if(kernel_weapon_action_api_v1->draw(weaponApiOwner_,api_.read<int>(Field::PlayerId,player),s->apiTick,&list)!=WA_OK)return;
 auto world=api_.read<Vec2>(Field::Position,player);
 auto screen=invokeStatic<Vec2>(api_.screen)-(api_.read<Vec2>(Field::DrawPosition,ctx)+api_.read<Vec2>(Field::VisualOffset,ctx)-world);
 for(uint32_t i=0;i<list.count;++i){auto &c=list.commands[i];if(c.layer!=layer)continue;
  Handle texture{};NativeRect source{};
  if(c.texture==WA_PIXEL_TEXTURE){texture=api_.assetTexture(api_.read<Handle>(Field::MagicPixel));source={0,0,1,1};}
  else {auto visual=itemVisual(api_,c.item_id);if(!visual)continue;texture=visual.texture;
   source=c.source.w && c.source.h?NativeRect{c.source.x,c.source.y,c.source.w,c.source.h}:visual.frame;
   int w=api_.read<int>(Field::TextureWidth,texture),h=api_.read<int>(Field::TextureHeight,texture);
   if(int64_t(source.x)+source.w>w || int64_t(source.y)+source.h>h)continue;
  }
  if(!texture)continue;
  auto slot=invoke<Handle>(api_.addDraw,ctx);
  if(!slot)continue;
  invoke<void>(api_.setDrawRect,slot,texture,vector(c.position)-screen,source,toNativeColor({c.color.r,c.color.g,c.color.b,c.color.a}),c.rotation,vector(c.origin),vector(c.scale),int(c.effects),0);
 }
}
}

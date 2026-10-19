#include "game_runtime.hpp"
#include "draw_bridge.hpp"
#include "weapons/weapon_registry.hpp"
namespace mr {
void GameRuntime::drawCarry(Handle ctx,Handle player,PlayerState &state,bool back){
 if(state.current.active || !state.seen || (state.carry.amount()>=.5f)!=back)return;
 auto profile=findWeapon(state.combatItemId);if(!profile)return;
 auto items=api_.read<Handle>(Field::ItemTextures);Handle asset{};
 if(!items || patchlib_array_length(items)<=static_cast<std::size_t>(profile->itemId) || !patchlib_array_at(items,profile->itemId,&asset))return;
 auto sword=api_.assetTexture(asset);if(!sword)return;
 auto item=invoke<Handle>(api_.heldItem,player);
 if(!item || api_.read<int>(Field::ItemType,item)!=profile->itemId)return;
 auto world=api_.read<Vec2>(Field::Position,player);
 auto translation=api_.read<Vec2>(Field::DrawPosition,ctx)+api_.read<Vec2>(Field::VisualOffset,ctx)-world;
 auto screen=invokeStatic<Vec2>(api_.screen)-translation;
 int direction=api_.read<int>(Field::Direction,player);
 float gravity=api_.read<float>(Field::Gravity,player);
 auto waist=waistGrip(world,api_.read<int>(Field::Width,player),api_.read<int>(Field::Height,player),direction,gravity);
 float t=state.carry.amount();t=t*t*(3-2*t);
 Vec2 grip=state.carry.ready()?state.current.grip:waist;
 float readyAngle=state.carry.ready()?std::atan2(state.current.tip.y-state.current.grip.y,
                       state.current.tip.x-state.current.grip.x):waistAngle(direction,gravity);
 float target=waistAngle(direction,gravity);
 float angle=readyAngle+std::remainder(target-readyAngle,6.283185307f)*t;
 grip=grip+(waist-grip)*t;
 float width=float(api_.read<int>(Field::TextureWidth,sword)),height=float(api_.read<int>(Field::TextureHeight,sword));
 if(width<5 || height<5 || width>4096 || height>4096)return;
 Vec2 axis{width-4,4-height};float scale=profile->bladeLength*api_.read<float>(Field::ItemScale,item)/length(axis);
 enqueueDraw(api_,ctx,sword,grip-screen,{255,255,255,255},angle-std::atan2(axis.y,axis.x),{2,height-2},{scale,scale});
}
void GameRuntime::drawBack(Handle ctx){
 if(!validScene() || !ctx || api_.read<float>(Field::Shadow,ctx)>0 ||
    api_.read<bool>(Field::DrawHidden,ctx) || api_.read<bool>(Field::DrawHiddenExcept,ctx))return;
 auto player=api_.read<Handle>(Field::DrawPlayer,ctx);
 if(!player || api_.read<bool>(Field::Dead,player) || !api_.read<bool>(Field::Active,player))return;
 auto state=this->state(player);if(!state)return;
 auto item=invoke<Handle>(api_.heldItem,player);if(!item)return;
 if(!findWeapon(api_.read<int>(Field::ItemType,item))){drawGenericCarry(ctx,player,*state,true);return;}
 if(resourcesReady())drawCarry(ctx,player,*state,true);
}
}

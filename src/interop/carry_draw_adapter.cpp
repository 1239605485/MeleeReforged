#include "game_runtime.hpp"
#include "draw_bridge.hpp"
#include "item_visuals.hpp"
#include "carry/pose.hpp"
#include "carry/whip_renderer.hpp"
namespace mr {
bool GameRuntime::drawGenericCarry(Handle ctx,Handle player,PlayerState &state,bool back){
 auto &carry=state.presentation;
 if(back){carry.backDrawn=false;carry.backContext=reinterpret_cast<std::uintptr_t>(ctx);}
 if(!api_.carryReady || !carry.visible())return false;
 auto item=invoke<Handle>(api_.heldItem,player);
 if(!item || api_.read<int>(Field::ItemType,item)!=carry.itemId ||
    api_.read<int>(Field::Animation,player)>0 || api_.read<bool>(Field::ControlUseItem,player))return false;
 auto visual=itemVisual(api_,carry.itemId);if(!visual)return false;
 auto world=api_.read<Vec2>(Field::Position,player);
 auto screen=invokeStatic<Vec2>(api_.screen)-
   (api_.read<Vec2>(Field::DrawPosition,ctx)+api_.read<Vec2>(Field::VisualOffset,ctx)-world);
 int direction=api_.read<int>(Field::Direction,player);float gravity=api_.read<float>(Field::Gravity,player);
 auto mount=api_.read<Handle>(Field::Mount,player);bool mounted=mount && invoke<bool>(api_.mountActive,mount);
 auto velocity=api_.read<Vec2>(Field::Velocity,player);
 float sway=std::sin(float(carry.lastTick%40)*6.283185307f/40)*std::min(1.f,std::abs(velocity.x)*.15f);
 auto pose=carriedPose(carry.profile,world,api_.read<int>(Field::Width,player),api_.read<int>(Field::Height,player),direction,gravity,sway,mounted);
 float t=carry.amount();t=t*t*(3-2*t);
 auto useOffset=carry.useOffset;
 if(direction!=carry.useDirection)useOffset.x=api_.read<int>(Field::Width,player)-useOffset.x;
 if(gravity!=carry.useGravity)useOffset.y=api_.read<int>(Field::Height,player)-useOffset.y;
 Vec2 ready=carry.hasUsePose?world+useOffset:world+Vec2{api_.read<int>(Field::Width,player)*.5f+direction*12.f,api_.read<int>(Field::Height,player)*.5f-5*gravity};
 float readyAngle=carry.hasUsePose?std::atan2(std::sin(carry.useAngle)*gravity/carry.useGravity,
                          std::cos(carry.useAngle)*direction/carry.useDirection):std::atan2(-gravity*.4f,float(direction));
 pose.position=ready+(pose.position-ready)*t;
 float angle=readyAngle+std::remainder(pose.angle-readyAngle,6.283185307f)*t;
 bool drawBehind=pose.back && t>=.5f;
 if(drawBehind!=back){
  if(back)return false;
  if(carry.backDrawn && carry.backContext==reinterpret_cast<std::uintptr_t>(ctx))return true;
  // If the back layer could not allocate draw data, render here as a fallback.
 }
 float itemScale=api_.read<float>(Field::ItemScale,item);
 if(!std::isfinite(itemScale) || itemScale<=0)return false;
 if(carry.profile.kind==CarryKind::Whip){
  auto pixel=api_.assetTexture(api_.read<Handle>(Field::MagicPixel));if(!pixel)return false;
  auto commands=coiledWhip(carry.profile,pose.position,direction,gravity,carry.profile.angle,std::clamp(itemScale,.5f,1.4f));
  for(std::size_t i=0;i<commands.count;++i){
   const auto &d=commands.data[i];auto delta=d.to-d.from;
   if(!enqueueDraw(api_,ctx,pixel,d.from-screen,d.color,std::atan2(delta.y,delta.x),{0,.5f},{length(delta),d.width},true))return false;
  }
 }else{
  float scale=std::min(itemScale,carry.profile.maxSpan/float(std::max(visual.frame.w,visual.frame.h)));
  auto slot=invoke<Handle>(api_.addDraw,ctx);if(!slot)return false;
  // Reflect the source axis before computing rotation; retain the item's aspect ratio.
  float sourceAngle=pose.effects? -carry.profile.sourceAngle:carry.profile.sourceAngle;
  invoke<void>(api_.setDrawRect,slot,visual.texture,pose.position-screen,visual.frame,toNativeColor({255,255,255,255}),
    angle-sourceAngle,Vec2{visual.frame.w*.5f,visual.frame.h*.5f},Vec2{scale,scale},pose.effects,0);
 }
 if(back)carry.backDrawn=true;
 return true;
}
}

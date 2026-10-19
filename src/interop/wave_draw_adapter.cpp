#include "game_runtime.hpp"
#include "draw_bridge.hpp"
#include "effect_atlas.hpp"
#include "render/wave_renderer.hpp"
namespace mr {
void GameRuntime::drawWaves(Handle ctx,const PlayerState &state,Vec2 screen,Handle pixel){
 auto atlas=effectAtlas(api_);
 for(const auto &wave:state.waves){
  if(!wave.active)continue;
  float fade=std::clamp(std::min(wave.age*30.f,(96.f-wave.distance)/24.f),0.f,1.f);
  if(atlas)for(int pass=0;pass<2;++pass){
   auto slot=invoke<Handle>(api_.addDraw,ctx);if(!slot)break;
   Vec2 center=wave.position-Vec2{wave.direction*18.f,0};
   int effects=(wave.direction<0?1:0)|(wave.gravity<0?2:0);
   auto alpha=static_cast<unsigned char>(fade*(pass?190:65));
   invoke<void>(api_.setDrawRect,slot,atlas,center-screen,NativeRect{480,pass?0:160,160,160},
      toNativeColor({255,205,90,alpha}),.011137f,Vec2{80,80},Vec2{20.f/45,20.f/45},effects,0);
  }
  auto commands=WaveRenderer::build(wave);
  for(std::size_t i=0;i<commands.count;++i){
   const auto &d=commands.data[i];auto v=d.to-d.from;
   enqueueDraw(api_,ctx,pixel,d.from-screen,d.color,std::atan2(v.y,v.x),{0,.5f},{length(v),d.width},true);
  }
 }
}
}

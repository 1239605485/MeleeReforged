#pragma once
#include "runtime_api.hpp"
#include "render/swing_renderer.hpp"
namespace mr {
inline bool enqueueDraw(RuntimeApi &a, Handle ctx, Handle texture, Vec2 pos,
                    Color color, float angle, Vec2 origin, Vec2 scale, bool pixel = false, int effects = 0) {
  if (!texture)
    return false;
  auto slot = invoke<Handle>(a.addDraw, ctx);
  if (!slot)
    return false;
  if(pixel)
    invoke<void>(a.setDrawRect,slot,texture,pos,NativeRect{0,0,1,1},
                 toNativeColor(color),angle,origin,scale,effects,0);
  else
    invoke<void>(a.setDraw,slot,texture,pos,toNativeColor(color),angle,origin,scale,effects,0);
  return true;
}
}

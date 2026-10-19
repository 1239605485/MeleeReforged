#include "game_runtime.hpp"
#include "draw_bridge.hpp"
#include "render/swing_renderer.hpp"
#include "render/slash_sheet.hpp"
#include "weapons/weapon_registry.hpp"
#include "effect_atlas.hpp"
namespace mr {
static Handle assetTexture(RuntimeApi &a, Handle asset) {
  return a.assetTexture(asset);
}
bool GameRuntime::resourcesReady() {
  if (renderFailed_)
    return false;
  auto items = api_.read<Handle>(Field::ItemTextures);
  Handle asset{};
  if (!items || patchlib_array_length(items) <= 24 ||
      !patchlib_array_at(items, 24, &asset))
    return false;
  auto pixel = api_.read<Handle>(Field::MagicPixel);
  return pixel && asset && assetTexture(api_, pixel) &&
         assetTexture(api_, asset);
}
bool GameRuntime::draw(Handle ctx) {
  if (!validScene() || !ctx ||
      api_.read<float>(Field::Shadow, ctx) > 0)
    return false;
  if (api_.read<bool>(Field::DrawHidden, ctx) ||
      api_.read<bool>(Field::DrawHiddenExcept, ctx))
    return false;
  auto p = api_.read<Handle>(Field::DrawPlayer, ctx);
  auto s = state(p);
  if (!s || api_.read<bool>(Field::Dead,p))
    return false;
  auto item = invoke<Handle>(api_.heldItem, p);
  if(!item)return false;
  if(api_.read<int>(Field::ItemType,item)!=24)
    return drawGenericCarry(ctx,p,*s,false);
  if(!s->seen || !resourcesReady())
    return false;
  auto items = api_.read<Handle>(Field::ItemTextures);
  Handle swordAsset{};
  if (!items || patchlib_array_length(items) <= 24 ||
      !patchlib_array_at(items, 24, &swordAsset))
    return false;
  auto sword = assetTexture(api_, swordAsset),
       pixel = assetTexture(api_, api_.read<Handle>(Field::MagicPixel));
  if (!sword || !pixel)
    return false;
  auto worldPosition = api_.read<Vec2>(Field::Position, p);
  auto translation = api_.read<Vec2>(Field::DrawPosition, ctx) +
                     api_.read<Vec2>(Field::VisualOffset, ctx) - worldPosition;
  auto screen = invokeStatic<Vec2>(api_.screen) - translation;
  drawWaves(ctx,*s,invokeStatic<Vec2>(api_.screen),pixel);
  if(!s->current.active){
    drawCarry(ctx,p,*s,false);
    return true;
  }
  auto &pose = s->current;
  auto atlas=effectAtlas(api_);
  const auto &weapon=*findWeapon(24);
  auto fx=slashForAttack(pose,pose.charged?weapon.heavy[pose.combo]:weapon.swings[pose.combo]);
  auto drawSheet=[&](bool halo) {
    if(!atlas || !fx.visible || (!pose.charged && pose.combo==2)) return;
    auto slot=invoke<Handle>(api_.addDraw,ctx);
    if(!slot) return;
    float rotation=std::atan2(std::sin(fx.rotation)*pose.gravity,
                              std::cos(fx.rotation)*pose.direction);
    float scale=fx.radius/45.f;
    float opacity=halo?fx.glow:fx.opacity*.85f;
    Color tint=pose.charged?Color{255,205,90,static_cast<unsigned char>(opacity*255)}:
                           Color{100,225,255,static_cast<unsigned char>(opacity*255)};
    // Reflection about the local blade axis handles facing and inverted gravity.
    int effects=((pose.direction*pose.gravity<0)!=fx.mirrored)?2:0;
    invoke<void>(api_.setDrawRect,slot,atlas,pose.grip-screen,
      NativeRect{fx.frame*160,halo?160:0,160,160},toNativeColor(tint),
      rotation,Vec2{80,80},Vec2{scale,scale},effects,0);
  };
  auto commands = SwingRenderer::build(pose, s->trail, Quality::Medium);
  auto drawEffects = [&](bool front) {
    for (std::size_t i = 0; i < commands.count; ++i) {
      auto &d = commands.data[i];
      if ((d.layer >= 3) != front)
        continue;
      auto v = d.to - d.from;
      enqueueDraw(api_, ctx, pixel, d.from - screen, d.color, std::atan2(v.y, v.x),
              {0, .5f}, {length(v), d.width}, true);
    }
  };
  drawSheet(true);
  drawSheet(false);
  drawEffects(false);
  const float width=float(api_.read<int>(Field::TextureWidth,sword));
  const float height=float(api_.read<int>(Field::TextureHeight,sword));
  if(width<5 || height<5 || width>4096 || height>4096) return false;
  const Vec2 bladeAxis{width-4,4-height};
  float angle=std::atan2(pose.tip.y-pose.grip.y,pose.tip.x-pose.grip.x)-
              std::atan2(bladeAxis.y,bladeAxis.x);
  float scale=length(pose.tip-pose.grip)/length(bladeAxis);
  if (!enqueueDraw(api_,ctx,sword,pose.grip-screen,{255,255,255,255},
               angle,{2,height-2},{scale,scale})) {
    renderFailure();return false;
  }
  drawEffects(true);
  auto now = invokeStatic<unsigned>(api_.tick);
  for (const auto &hit : s->impacts) {
    if (!hit.active || now < hit.tick || now - hit.tick > 12)
      continue;
    float age = float(now - hit.tick) / 60.f, fade = 1.f - age / .2f;
    for (int i = 0; i < 6; ++i) {
      float a = float(i) * 1.04719755f + float(hit.tick % 10) * .1f;
      Vec2 v{std::cos(a), std::sin(a)};
      auto pos = hit.position + v * (age * 100.f) - screen;
      enqueueDraw(api_, ctx, pixel, pos,
              {150, 255, 190, static_cast<unsigned char>(fade * 220)}, a,
              {0, .5f}, {4.f * fade, 2.f * fade}, true);
    }
  }
  return true; // Suppress only this wooden sword's original held-item layer.
}
} // namespace mr

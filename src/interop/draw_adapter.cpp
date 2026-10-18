#include "game_runtime.hpp"
#include "render/swing_renderer.hpp"
namespace mr {
static Handle assetTexture(RuntimeApi &a, Handle asset) {
  return a.assetTexture(asset);
}
static bool enqueue(RuntimeApi &a, Handle ctx, Handle texture, Vec2 pos,
                    Color color, float angle, Vec2 origin, Vec2 scale) {
  if (!texture)
    return false;
  auto slot = invoke<Handle>(a.addDraw, ctx);
  if (!slot)
    return false;
  invoke<void>(a.setDraw, slot, texture, pos, premultiply(color), angle, origin,
               scale, 0, 0);
  return true;
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
  if (!validScene() || !ctx || !resourcesReady() ||
      api_.read<float>(Field::Shadow, ctx) > 0)
    return false;
  if (api_.read<bool>(Field::DrawHidden, ctx) ||
      api_.read<bool>(Field::DrawHiddenExcept, ctx))
    return false;
  auto p = api_.read<Handle>(Field::DrawPlayer, ctx);
  auto s = state(p);
  if (!s || !s->current.active)
    return false;
  auto item = invoke<Handle>(api_.heldItem, p);
  if (!item || api_.read<int>(Field::ItemType, item) != 24)
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
  auto &pose = s->current;
  auto commands = SwingRenderer::build(pose, s->trail, Quality::Medium);
  auto drawEffects = [&](bool front) {
    for (std::size_t i = 0; i < commands.count; ++i) {
      auto &d = commands.data[i];
      if ((d.layer >= 3) != front)
        continue;
      auto v = d.to - d.from;
      enqueue(api_, ctx, pixel, d.from - screen, d.color, std::atan2(v.y, v.x),
              {0, .5f}, {length(v), d.width});
    }
  };
  drawEffects(false);
  float angle = std::atan2(pose.tip.y - pose.grip.y, pose.tip.x - pose.grip.x) +
                .785398163f;
  float scale = api_.read<float>(Field::ItemScale, item);
  if (!enqueue(api_, ctx, sword, pose.grip - screen, {255, 255, 255, 255},
               angle, {8, 26}, {scale, scale})) {
    renderFailure();
    return false;
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
      enqueue(api_, ctx, pixel, pos,
              {150, 255, 190, static_cast<unsigned char>(fade * 220)}, a,
              {0, .5f}, {4.f * fade, 2.f * fade});
    }
  }
  return true; // Suppress only this wooden sword's original held-item layer.
}
} // namespace mr

#include "combat/trajectory.hpp"
#include "game_runtime.hpp"
#include <algorithm>
namespace mr {
bool GameRuntime::validScene() {
  return ready_.load() && !invokeStatic<bool>(api_.menu) &&
         api_.read<int>(Field::NetMode) == 0;
}
void GameRuntime::frame() {
  if (!ready_.load())
    return;
  bool inWorld =
      !invokeStatic<bool>(api_.menu) && api_.read<int>(Field::NetMode) == 0;
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
  if (!item || api_.read<int>(Field::ItemType, item) != 24 ||
      api_.read<bool>(Field::Dead, p) || !api_.read<bool>(Field::Active, p)) {
    s->clear();
    return;
  }
  auto tick = invokeStatic<unsigned>(api_.tick);
  // A no-use tick still completes recovery, ages trails and advances combo idle
  // timeout.
  if (!s->seen || s->lastTick != tick)
    action(p);
}
void GameRuntime::action(Handle p) {
  if (!validScene() || api_.read<bool>(Field::Paused) || !resourcesReady())
    return;
  auto s = state(p);
  if (!s)
    return;
  auto item = invoke<Handle>(api_.heldItem, p);
  if (!item || api_.read<int>(Field::ItemType, item) != 24 ||
      api_.read<bool>(Field::Dead, p) || !api_.read<bool>(Field::Active, p)) {
    s->clear();
    return;
  }
  auto now = invokeStatic<unsigned>(api_.tick);
  if (s->seen && now == s->lastTick) {
    if (s->current.active)
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
  in.itemId = 24;
  in.direction = api_.read<int>(Field::Direction, p);
  in.gravity = api_.read<float>(Field::Gravity, p);
  in.cycleSeconds = std::max(1, api_.read<int>(Field::AnimationMax, p)) / 60.f;
  in.dtSeconds = dt;
  in.grip = pos + Vec2{api_.read<int>(Field::Width, p) * .5f,
                       api_.read<int>(Field::Height, p) * .4f};
  in.bladeLength = 31.f * scale;
  in.attacking = api_.read<int>(Field::Animation, p) > 0;
  in.alive = true;
  in.inWorld = true;
  in.singlePlayer = true;
  s->previous = s->current;
  s->current = s->controller.advance(in);
  s->lastTick = now;
  s->seen = true;
  auto &pose = s->current;
  if (!pose.active) {
    s->trail.clear();
    return;
  }
  apply(p, *s);
  s->trail.push(now, pose);
}
void GameRuntime::apply(Handle p, PlayerState &s) {
  auto &pose = s.current;
  float worldAngle =
      std::atan2(pose.tip.y - pose.grip.y, pose.tip.x - pose.grip.x);
  float armAngle = worldAngle - 1.570796327f;
  invoke<void>(api_.arm, p, true, 3, armAngle);
  auto grip = invoke<Vec2>(api_.hand, p, 3, armAngle);
  pose.tip = pose.tip + (grip - pose.grip);
  pose.grip = grip;
  api_.write(Field::Location, p, grip);
  api_.write(Field::Rotation, p, worldAngle);
}
} // namespace mr

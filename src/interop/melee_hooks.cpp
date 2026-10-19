#include "collision/damage_sweep.hpp"
#include "combat/trajectory.hpp"
#include "game_runtime.hpp"
#include "weapons/wooden_sword_rules.hpp"
#include <algorithm>
namespace mr {
static Rect rectOf(RuntimeApi &a, Handle npc) {
  auto p = a.read<Vec2>(Field::Position, npc);
  return {p.x, p.y, float(a.read<int>(Field::Width, npc)),
          float(a.read<int>(Field::Height, npc))};
}
bool GameRuntime::hitbox(Handle p, void **args) {
  if (!validScene() || !resourcesReady())
    return false;
  auto s = state(p);
  if (!s || !s->current.active || !args || !args[0])
    return false;
  auto item = *static_cast<Handle *>(args[0]);
  if (!item || api_.read<int>(Field::ItemType, item) != 24)
    return false;
  if (!args[2] || !args[3])
    return false;
  auto dont = *static_cast<bool **>(args[2]);
  auto rectangle = *static_cast<NativeRect **>(args[3]);
  if (!dont || !rectangle)
    return false;
  auto sweep = collectDamageSweep(s->previous, s->current);
  *dont = sweep.count < 2;
  float minX = s->current.grip.x, minY = s->current.grip.y, maxX = minX,
        maxY = minY;
  for (std::size_t i = 0; i < sweep.count; ++i) {
    auto v = sweep.segments[i];
    minX = std::min({minX, v.grip.x, v.tip.x});
    minY = std::min({minY, v.grip.y, v.tip.y});
    maxX = std::max({maxX, v.grip.x, v.tip.x});
    maxY = std::max({maxY, v.grip.y, v.tip.y});
  }
  *rectangle = {int(std::floor(minX)) - 1, int(std::floor(minY)) - 1,
                int(std::ceil(maxX - minX)) + 3,
                int(std::ceil(maxY - minY)) + 3};
  return true;
}
bool GameRuntime::hitNpc(Handle p, void **args) {
  if (!validScene() || !resourcesReady())
    return false;
  auto s = state(p);
  if (!s || !s->current.active || !args)
    return false;
  auto item = *static_cast<Handle *>(args[0]);
  if (!item || api_.read<int>(Field::ItemType, item) != 24)
    return false;
  s->pendingNpc = 0;
  auto sweep = collectDamageSweep(s->previous, s->current);
  if (sweep.count < 2)
    return true;
  int id = *static_cast<int *>(args[4]);
  auto npcs = api_.read<Handle>(Field::Npcs);
  Handle npc{};
  if (!npcs || id < 0 ||
      static_cast<std::size_t>(id) >= patchlib_array_length(npcs) ||
      !patchlib_array_at(npcs, id, &npc) || !npc)
    return true;
  if (!api_.read<bool>(Field::NpcActive, npc))
    return true;
  auto generation = npcGenerations_[id];
  if (!s->damage.shouldProcess(sweep.attackId, id, generation))
    return true;
  bool intersects = false;
  auto target = rectOf(api_, npc);
  for (std::size_t i = 1; i < sweep.count; ++i)
    if (sweepIntersects(sweep.segments[i - 1], sweep.segments[i], target)) {
      intersects = true;
      break;
    }
  if (!intersects)
    return true;
  s->pendingNpc = reinterpret_cast<std::uintptr_t>(npc);
  s->pendingLife = api_.read<int>(Field::NpcLife, npc);
  s->pendingSlot = id;
  s->pendingAttack = sweep.attackId;
  const auto &attack=sweep.attackId==s->current.attackId?s->current:s->previous;
  auto power=woodenMeleePower(attack.charged,attack.combo);
  auto &damage=*static_cast<int *>(args[2]);
  auto &knockback=*static_cast<float *>(args[3]);
  damage=scaleDamage(damage,power.damage);
  knockback*=power.knockback;
  return false;
}
void GameRuntime::afterHit(Handle p, void **args) {
  if (!validScene() || !args)
    return;
  auto s = state(p);
  if (!s || !s->pendingNpc)
    return;
  int id = *static_cast<int *>(args[4]);
  if (id != s->pendingSlot)
    return;
  auto npc = reinterpret_cast<Handle>(s->pendingNpc);
  if (api_.read<int>(Field::NpcLife, npc) < s->pendingLife) {
    s->damage.markProcessed(s->pendingAttack, id, npcGenerations_[id]);
    auto r = rectOf(api_, npc);
    impact(*s, {r.x + r.w * .5f, r.y + r.h * .5f});
  }
  s->pendingNpc = 0;
  s->pendingSlot = -1;
}
void GameRuntime::impact(PlayerState &s, Vec2 p) {
  s.hitConfirmed=true;
  s.impacts[s.nextImpact++ % s.impacts.size()] = {p, s.lastTick, true};
}
} // namespace mr

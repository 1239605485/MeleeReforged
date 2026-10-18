#include "collision/blade_sweep.hpp"
#include "collision/damage_sweep.hpp"
#include "combat/attack_controller.hpp"
#include "combat/player_states.hpp"
#include "render/swing_renderer.hpp"
#include "weapons/weapon_registry.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
using namespace mr;
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      std::fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x);             \
      std::exit(1);                                                            \
    }                                                                          \
  } while (0)
int main() {
  CHECK(findWeapon(24));
  CHECK(!findWeapon(25));
  AttackController c;
  AttackInput in{};
  in.itemId = 24;
  in.alive = true;
  in.inWorld = true;
  in.singlePlayer = true;
  in.attacking = true;
  in.cycleSeconds = .4f;
  in.direction = 1;
  in.gravity = 1;
  in.bladeLength = 30;
  auto p = c.advance(in);
  CHECK(p.active);
  CHECK(p.combo == 0);
  CHECK(!p.damaging);
  in.dtSeconds = .16f;
  in.tick++;
  p = c.advance(in);
  CHECK(p.damaging);
  auto same = c.advance(in);
  CHECK(same.progress == p.progress);
  in.dtSeconds = .24f;
  in.tick++;
  p = c.advance(in);
  CHECK(p.combo == 1);
  in.dtSeconds = .4f;
  in.tick++;
  p = c.advance(in);
  CHECK(p.combo == 2);
  in.tick++;
  p = c.advance(in);
  CHECK(p.combo == 0);
  in.itemId = 25;
  in.tick++;
  CHECK(!c.advance(in).active);
  in.itemId = 24;
  in.tick++;
  CHECK(c.advance(in).combo == 0);
  in.worldGeneration++;
  in.tick++;
  CHECK(c.advance(in).combo == 0);
  in.playerGeneration++;
  in.tick++;
  CHECK(c.advance(in).combo == 0);
  in.alive = false;
  in.tick++;
  CHECK(!c.advance(in).active);
  in.alive = true;
  in.singlePlayer = false;
  in.tick++;
  CHECK(!c.advance(in).active);
  in.singlePlayer = true;
  in.dtSeconds = std::numeric_limits<float>::quiet_NaN();
  in.tick++;
  CHECK(!c.advance(in).active);
  AttackController a, b;
  in.dtSeconds = 0;
  in.cycleSeconds = .4f;
  in.direction = 1;
  in.tick++;
  a.advance(in);
  in.direction = -1;
  b.advance(in);
  in.dtSeconds = .16f;
  in.tick++;
  in.direction = 1;
  auto pa = a.advance(in);
  in.direction = -1;
  auto pb = b.advance(in);
  CHECK(std::abs(pa.tip.x + pb.tip.x) < .001f);
  CHECK(std::abs(pa.tip.y - pb.tip.y) < .001f);
  AttackController slow, fast;
  in.direction = 1;
  in.tick++;
  in.dtSeconds = 0;
  slow.advance(in);
  in.cycleSeconds = .2f;
  fast.advance(in);
  in.tick++;
  in.dtSeconds = .08f;
  auto pf = fast.advance(in);
  in.cycleSeconds = .4f;
  in.dtSeconds = .16f;
  auto ps = slow.advance(in);
  CHECK(std::abs(pf.angle - ps.angle) < .001f);
  BladeSegment s0{{0, 0}, {30, 0}}, s1{{0, 0}, {0, 30}};
  CHECK(sweepIntersects(s0, s1, {10, 10, 2, 2}));
  CHECK(!sweepIntersects(s0, s1, {27, 27, 2, 2}));
  DamageWindow window;
  CHECK(window.shouldProcess(1, 2, 0));
  window.markProcessed(1, 2, 0);
  CHECK(!window.shouldProcess(1, 2, 0));
  CHECK(window.shouldProcess(2, 2, 0));
  CHECK(window.shouldProcess(1, 2, 1));
  TrailBuffer trail;
  pa.tick = 5;
  trail.push(pa.tick, pa);
  trail.push(pa.tick, pa);
  CHECK(trail.size() == 1);
  trail.clear();
  CHECK(trail.size() == 0);
  EffectEvents events;
  CHECK(events.emitOnce(7));
  CHECK(!events.emitOnce(7));
  events.clear();
  CHECK(events.emitOnce(7));
  for (auto q : {Quality::Low, Quality::Medium, Quality::High}) {
    auto cmd = SwingRenderer::build(pa, trail, q);
    CHECK(cmd.size() > 0);
    CHECK(cmd.size() <= 320);
  }
  auto before = evaluatePose(*findWeapon(24), 0, .1f, {0, 0}, 1, 1, 30);
  before.attackId = 50;
  auto after = evaluatePose(*findWeapon(24), 0, .9f, {0, 0}, 1, 1, 30);
  after.attackId = 50;
  auto sweep = collectDamageSweep(before, after);
  CHECK(sweep.count > 2);
  CHECK(sweep.attackId == 50);
  auto recover = evaluatePose(*findWeapon(24), 0, .95f, {0, 0}, 1, 1, 30);
  recover.attackId = 50;
  CHECK(collectDamageSweep(after, recover).count == 0);
  PlayerStates players;
  players.get(0)->identity = 5;
  CHECK(players.get(1)->identity == 0);
  CHECK(!players.get(-1));
  CHECK(!players.get(256));
  players.clear();
  CHECK(players.get(0)->identity == 0);
  AttackController idle;
  in.dtSeconds = 0;
  in.tick++;
  in.cycleSeconds = .2f;
  in.attacking = true;
  idle.advance(in);
  in.tick++;
  in.attacking = false;
  in.dtSeconds = .2f;
  CHECK(!idle.advance(in).active);
  in.tick++;
  in.dtSeconds = .61f;
  idle.advance(in);
  in.tick++;
  in.dtSeconds = 0;
  in.attacking = true;
  CHECK(idle.advance(in).combo == 0);
  auto white = premultiply({100, 200, 240, 128});
  CHECK(white.r == 50);
  CHECK(white.g == 100);
  CHECK(white.b == 120);
  std::puts("core checks passed");
}

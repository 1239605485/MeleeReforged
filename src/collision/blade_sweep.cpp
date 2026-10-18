#include "blade_sweep.hpp"
#include <algorithm>
namespace mr {
static float cross(Vec2 a, Vec2 b) { return a.x * b.y - a.y * b.x; }
static bool inside(Vec2 p, const Rect &r) {
  return p.x >= r.x && p.x <= r.x + r.w && p.y >= r.y && p.y <= r.y + r.h;
}
static bool line(Vec2 a, Vec2 b, Vec2 c, Vec2 d) {
  auto v = b - a, w = d - c;
  float det = cross(v, w);
  if (std::abs(det) < 1e-6f)
    return false;
  float t = cross(c - a, w) / det, u = cross(c - a, v) / det;
  return t >= 0 && t <= 1 && u >= 0 && u <= 1;
}
static bool segment(Vec2 a, Vec2 b, const Rect &r) {
  if (inside(a, r) || inside(b, r))
    return true;
  Vec2 p[4] = {
      {r.x, r.y}, {r.x + r.w, r.y}, {r.x + r.w, r.y + r.h}, {r.x, r.y + r.h}};
  for (int i = 0; i < 4; ++i) {
    if (line(a, b, p[i], p[(i + 1) % 4]))
      return true;
  }
  return false;
}
static bool triangle(Vec2 a, Vec2 b, Vec2 c, const Rect &r) {
  if (segment(a, b, r) || segment(b, c, r) || segment(c, a, r))
    return true;
  Vec2 p{r.x, r.y};
  float area = cross(b - a, c - a);
  if (std::abs(area) < 1e-6f)
    return false;
  float x = cross(b - a, p - a), y = cross(c - b, p - b),
        z = cross(a - c, p - c);
  return (x >= 0 && y >= 0 && z >= 0) || (x <= 0 && y <= 0 && z <= 0);
}
bool sweepIntersects(const BladeSegment &a, const BladeSegment &b,
                     const Rect &r) {
  if (r.w < 0 || r.h < 0)
    return false;
  return triangle(a.grip, a.tip, b.tip, r) ||
         triangle(a.grip, b.tip, b.grip, r);
}
bool DamageWindow::shouldProcess(std::uint64_t a, int n,
                                 std::uint64_t g) const {
  if (n < 0 || n >= 200)
    return false;
  const auto &e = hits_[n];
  return !e.set || e.attack != a || e.generation != g;
}
void DamageWindow::markProcessed(std::uint64_t a, int n, std::uint64_t g) {
  if (n >= 0 && n < 200)
    hits_[n] = {a, g, true};
}
} // namespace mr

#include "swing_renderer.hpp"
#include <algorithm>
namespace mr {
DrawCommands SwingRenderer::build(const AttackPose &p, const TrailBuffer &t,
                                  Quality q) {
  DrawCommands out;
  if (!p.active)
    return out;
  auto max = budget(q).points;
  std::size_t begin = t.size() > max ? t.size() - max : 0;
  for (std::size_t i = begin; i < t.size(); ++i) {
    auto &v = t[i];
    float life = v.pose.combo == 2 ? .18f : .12f;
    float fade = std::max(0.f, 1 - (t.time() - v.time) / life);
    Vec2 inner = v.pose.grip + (v.pose.tip - v.pose.grip) * .48f;
    unsigned char a = static_cast<unsigned char>(fade * 125);
    out.add({inner, v.pose.tip, p.combo == 2 ? 9.f : 5.f, {18, 165, 82, a}, 0});
    if (i > begin) {
      out.add({t[i - 1].pose.tip,
               v.pose.tip,
               p.combo == 2 ? 5.f : 3.f,
               {80, 255, 170, a},
               1});
      if (p.combo == 1)
        out.add({t[i - 1].pose.grip +
                     (t[i - 1].pose.tip - t[i - 1].pose.grip) * .8f,
                 inner + (v.pose.tip - inner) * .6f,
                 2.f,
                 {180, 255, 210, a},
                 1});
    }
  }
  // Commands contain effects only. The vanilla weapon entity remains drawn by
  // its own player layer.
  out.add(
      {p.grip + (p.tip - p.grip) * .88f, p.tip, 1.5f, {190, 255, 220, 170}, 3});
  return out;
}
} // namespace mr

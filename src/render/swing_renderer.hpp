#pragma once
#include "config/quality.hpp"
#include "trail_buffer.hpp"
namespace mr {
struct Color {
  unsigned char r{}, g{}, b{}, a{};
};
inline Color premultiply(Color c) {
  return {static_cast<unsigned char>(int(c.r) * c.a / 255),
          static_cast<unsigned char>(int(c.g) * c.a / 255),
          static_cast<unsigned char>(int(c.b) * c.a / 255), c.a};
}
// Android XNA Color is laid out A,B,G,R (verified against the supplied dump).
struct NativeColor { unsigned char a,b,g,r; };
inline NativeColor toNativeColor(Color c) {
  c=premultiply(c);return {c.a,c.b,c.g,c.r};
}
struct DrawCommand {
  Vec2 from, to;
  float width{};
  Color color{};
  int layer{};
};
struct DrawCommands {
  std::array<DrawCommand, 320> data{};
  std::size_t count{};
  void add(DrawCommand d) {
    if (count < data.size())
      data[count++] = d;
  }
  std::size_t size() const { return count; }
};
class SwingRenderer {
public:
  static DrawCommands build(const AttackPose &, const TrailBuffer &, Quality);
};
} // namespace mr

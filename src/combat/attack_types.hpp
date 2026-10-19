#pragma once
#include <array>
#include <cmath>
#include <cstdint>
namespace mr {
struct Vec2 {
  float x{}, y{};
};
inline Vec2 operator+(Vec2 a, Vec2 b) { return {a.x + b.x, a.y + b.y}; }
inline Vec2 operator-(Vec2 a, Vec2 b) { return {a.x - b.x, a.y - b.y}; }
inline Vec2 operator*(Vec2 a, float t) { return {a.x * t, a.y * t}; }
inline float length(Vec2 a) { return std::sqrt(a.x * a.x + a.y * a.y); }
struct Rect {
  float x{}, y{}, w{}, h{};
};
struct BladeSegment {
  Vec2 grip, tip;
};
enum class Phase { Idle, Charging, Windup, Swing, Recovery };
struct AttackInput {
  std::uint64_t tick{}, worldGeneration{}, playerGeneration{};
  int itemId{}, direction{1};
  float gravity{1}, cycleSeconds{.4f}, dtSeconds{}, bladeLength{30};
  Vec2 grip{};
  bool attacking{}, alive{}, inWorld{}, singlePlayer{};
  bool charging{}, chargedAttack{};
  bool hitConfirmed{};
  float chargeLevel{};
};
struct AttackPose {
  std::uint64_t attackId{}, tick{};
  int combo{};
  Phase phase{Phase::Idle};
  float progress{}, angle{}, ageSeconds{}, bladeLength{30};
  Vec2 grip{}, tip{}, handOffset{};
  bool damaging{}, active{}, charging{}, charged{};
  float chargeLevel{}, bodyLean{}, bodyDrop{}, bodyShift{}, legLean{}, legShift{};
  float armAngle{}, backArmAngle{};
  int direction{1};
  float gravity{1};
};
struct SwingProfile {
  float windup, swing, recovery, startAngle, endAngle;
};
struct WeaponProfile {
  int itemId;
  std::array<SwingProfile, 3> swings;
  std::array<SwingProfile, 3> heavy;
};
} // namespace mr

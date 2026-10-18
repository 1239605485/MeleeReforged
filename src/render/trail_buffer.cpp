#include "trail_buffer.hpp"
namespace mr {
void TrailBuffer::clear() {
  count_ = 0;
  tick_ = ~std::uint64_t(0);
  time_ = 0;
}
void TrailBuffer::push(std::uint64_t tick, const AttackPose &p) {
  if (tick == tick_)
    return;
  if (tick_ != ~std::uint64_t(0))
    time_ += static_cast<float>(tick - tick_) / 60.f;
  tick_ = tick;
  float life = p.combo == 2 ? .18f : .12f;
  std::size_t n = 0;
  for (std::size_t i = 0; i < count_; ++i)
    if (time_ - points_[i].time <= life &&
        points_[i].pose.attackId == p.attackId)
      points_[n++] = points_[i];
  count_ = n;
  if (!p.active || !p.damaging)
    return;
  if (count_ == points_.size()) {
    for (std::size_t i = 1; i < count_; ++i)
      points_[i - 1] = points_[i];
    --count_;
  }
  points_[count_++] = {p, time_};
}
bool EffectEvents::emitOnce(std::uint64_t k) {
  for (std::size_t i = 0; i < count_; ++i)
    if (keys_[i] == k)
      return false;
  keys_[next_] = k;
  next_ = (next_ + 1) % keys_.size();
  if (count_ < keys_.size())
    ++count_;
  return true;
}
} // namespace mr

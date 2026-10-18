#pragma once
#include <array>
#include <cstddef>
namespace mr {
struct HookRequest {
  void *target{};
  void *callback{};
  void **original{};
};
class HookSet {
  using Install = int (*)(void *, void *, void **);
  using Remove = int (*)(void *);
  Install install_{};
  Remove remove_{};
  std::array<void *, 8> targets_{};
  std::size_t count_{};
  bool failed_{};

public:
  HookSet(Install i, Remove r) : install_(i), remove_(r) {}
  bool install(const HookRequest &r) {
    if (failed_ || !install_ || !remove_ || !r.target || !r.callback ||
        !r.original || count_ == targets_.size())
      return false;
    for (std::size_t i = 0; i < count_; ++i)
      if (targets_[i] == r.target)
        return false;
    if (install_(r.target, r.callback, r.original) != 0 || !*r.original) {
      failed_ = true;
      reset();
      return false;
    }
    targets_[count_++] = r.target;
    return true;
  }
  bool reset() {
    std::array<void *, 8> remaining{};
    std::size_t n = 0;
    for (std::size_t i = count_; i > 0; --i)
      if (remove_(targets_[i - 1]) != 0)
        remaining[n++] = targets_[i - 1];
    targets_ = remaining;
    count_ = n;
    if (n)
      failed_ = true;
    return n == 0;
  }
  std::size_t count() const { return count_; }
  bool failed() const { return failed_; }
};
} // namespace mr

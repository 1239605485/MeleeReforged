#pragma once
#include <cstddef>
namespace mr {
enum class Quality { Low, Medium, High };
struct Budget {
  std::size_t points, particles;
};
constexpr Budget budget(Quality q) {
  return q == Quality::Low      ? Budget{12, 24}
         : q == Quality::Medium ? Budget{20, 48}
                                : Budget{28, 80};
}
} // namespace mr

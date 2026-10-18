#include "weapon_registry.hpp"
namespace mr {
const WeaponProfile &woodenSword() {
  static const WeaponProfile p{24,
                               {{{.20f, .45f, .35f, -2.2f, 1.0f},
                                 {.15f, .45f, .40f, 1.2f, -2.0f},
                                 {.30f, .35f, .35f, -2.5f, .75f}}}};
  return p;
}
} // namespace mr

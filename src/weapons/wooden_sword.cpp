#include "weapon_registry.hpp"
namespace mr {
const WeaponProfile &woodenSword() {
  static const WeaponProfile p{24,
    {{{.24f,.27f,.49f,-2.15f,.92f},
      {.18f,.25f,.57f,1.10f,-1.85f},
      {.32f,.24f,.44f,-.06f,.02f}}},
    {{{.28f,.26f,.46f,-2.50f,1.15f},
      {.22f,.30f,.48f,1.30f,-.35f},
      {.26f,.27f,.47f,1.20f,-1.95f}}}};
  return p;
}
}

#include "weapon_registry.hpp"
namespace mr {
const WeaponProfile *findWeapon(int id) {
  return id == 24 ? &woodenSword() : (id == 2273 ? &katana() : nullptr);
}
} // namespace mr

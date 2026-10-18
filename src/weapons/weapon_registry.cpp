#include "weapon_registry.hpp"
namespace mr {
const WeaponProfile *findWeapon(int id) {
  return id == 24 ? &woodenSword() : nullptr;
}
} // namespace mr

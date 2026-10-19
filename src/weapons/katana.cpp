#include "weapon_registry.hpp"
namespace mr {
const WeaponProfile &katana(){
 static const WeaponProfile p{2273,
 {{{.20f,.25f,.55f,-1.60f,.85f},{.18f,.27f,.55f,1.05f,-1.65f},{.28f,.18f,.54f,-1.35f,.25f}}},
 {{{.30f,.24f,.46f,2.35f,-.18f},{.30f,.24f,.46f,2.35f,-.18f},{.30f,.24f,.46f,2.35f,-.18f}}},
 34.f,{{.85f,.94f,1.18f}},1.35f,1};return p;
}
}

#include "weapon_registry.hpp"
namespace mr {
const WeaponProfile &katana(){
 static const WeaponProfile p{2273,
 {{{.20f,.25f,.55f,-1.25f,.60f},{.18f,.27f,.55f,.72f,-.95f},{.28f,.18f,.54f,-.65f,.38f}}},
 {{{.30f,.24f,.46f,-1.45f,.45f},{.30f,.24f,.46f,-1.45f,.45f},{.30f,.24f,.46f,-1.45f,.45f}}},
 34.f,{{.85f,.94f,1.18f}},1.35f,1};return p;
}
}

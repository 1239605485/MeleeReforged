#include "weapon_registry.hpp"
namespace mr {
const WeaponProfile &katana(){
 static const SwingProfile cut{.24f,.20f,.56f,2.35f,-.22f};
 static const SwingProfile iai{.12f,.22f,.66f,2.35f,-.30f};
 static const WeaponProfile p{2273,{{cut,cut,cut}},{{iai,iai,iai}},
 34.f,{{1.f,1.f,1.f}},1.4f,1,1};return p;
}
}

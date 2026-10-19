#pragma once
#include "combat/attack_types.hpp"
#include <array>
namespace mr {
enum class CarryKind {None,Blade,GreatBlade,Spear,Whip,Handgun,LongGun,HeavyGun,Bow,Staff,Book,Pickaxe,Axe,Hammer,MultiTool};
struct CarryTraits {
 int itemId{},damage{},pick{},axe{},hammer{},useStyle{};
 bool melee{},ranged{},magic{},summon{},staff{},arrow{},bullet{},consumable{},accessory{},noUseGraphic{},channel{};
};
struct CarryProfile {
 CarryKind kind{CarryKind::None};Vec2 offset{};float angle{},sourceAngle{},maxSpan{52};bool back{};
 std::array<unsigned char,3> rope{170,120,65};
 bool valid()const{return kind!=CarryKind::None;}
};
bool isVanillaWhip(int itemId);
CarryProfile carryProfile(const CarryTraits &,int frameWidth,int frameHeight);
}

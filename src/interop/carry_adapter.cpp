#include "game_runtime.hpp"
#include "item_visuals.hpp"
namespace mr {
void GameRuntime::updateGenericCarry(Handle player,PlayerState &state,Handle item){
 if(!api_.carryReady || api_.read<bool>(Field::Paused))return;
 auto now=invokeStatic<unsigned>(api_.tick);
 auto &carry=state.presentation;
 if(carry.lastTick==now && carry.itemId==api_.read<int>(Field::ItemType,item))return;
 float dt=carry.lastTick!=~std::uint64_t(0) && now>carry.lastTick?std::min(.1f,float(now-carry.lastTick)/60.f):0;
 CarryTraits t;
 t.itemId=api_.read<int>(Field::ItemType,item);t.damage=api_.read<int>(Field::ItemDamage,item);
 t.pick=api_.read<int>(Field::ItemPick,item);t.axe=api_.read<int>(Field::ItemAxe,item);t.hammer=api_.read<int>(Field::ItemHammer,item);
 t.useStyle=api_.read<unsigned char>(Field::ItemUseStyle,item);
 t.melee=api_.read<bool>(Field::ItemMelee,item);t.ranged=api_.read<bool>(Field::ItemRanged,item);
 t.magic=api_.read<bool>(Field::ItemMagic,item);t.summon=api_.read<bool>(Field::ItemSummon,item);
 t.consumable=api_.read<bool>(Field::ItemConsumable,item);t.accessory=api_.read<bool>(Field::ItemAccessory,item);
 t.noUseGraphic=api_.read<bool>(Field::ItemNoUseGraphic,item);t.channel=api_.read<bool>(Field::ItemChannel,item);
 int ammo=api_.read<int>(Field::ItemUseAmmo,item),arrow=api_.read<int>(Field::AmmoArrow),bullet=api_.read<int>(Field::AmmoBullet);
 t.arrow=arrow>0 && ammo==arrow;t.bullet=bullet>0 && ammo==bullet;
 auto staff=api_.read<Handle>(Field::ItemStaffSet);
 if(staff && t.itemId>0 && static_cast<std::size_t>(t.itemId)<patchlib_array_length(staff))patchlib_array_at(staff,t.itemId,&t.staff);
 auto visual=itemVisual(api_,t.itemId);
 auto profile=visual?carryProfile(t,visual.frame.w,visual.frame.h):CarryProfile{};
 bool usingNow=api_.read<int>(Field::Animation,player)>0 || api_.read<bool>(Field::ControlUseItem,player);
 carry.observe(t.itemId,profile.valid(),usingNow,now,dt);carry.profile=profile;
 if(usingNow && profile.valid()){
  auto offset=api_.read<Vec2>(Field::Location,player)-api_.read<Vec2>(Field::Position,player);
  float angle=api_.read<float>(Field::Rotation,player);
  if(std::isfinite(offset.x) && std::isfinite(offset.y) && length(offset)<160 && std::isfinite(angle)){
   carry.useOffset=offset;carry.useAngle=angle;carry.hasUsePose=true;
   carry.useDirection=api_.read<int>(Field::Direction,player)<0?-1:1;
   carry.useGravity=api_.read<float>(Field::Gravity,player)<0?-1.f:1.f;
  }
 }
}
}

#pragma once
#include "combat/attack_types.hpp"
#include "patchlib/field.h"
#include "patchlib/method.h"
#include "patchlib/struct/array.h"
#include <array>
#include <initializer_list>
#include <cstring>
namespace mr {
using Handle = patch_handle_t;
enum class FailureCode {
  None,
  MissingExports,
  SignatureMismatch,
  MissingField,
  MissingPointer
};
struct ResolveReport {
  bool compatible{};
  FailureCode failure{};
  const char *detail{};
};
enum class Field : std::size_t {
  ControlUseItem,
  ControlDown,
  BodyRotation,
  BodyPosition,
  LegRotation,
  LegPosition,
  FrontArm,
  BackArm,
  Velocity,
  Mount,
  Pulley,
  GrapCount,
  Frozen,
  Stoned,
  TextureWidth,
  TextureHeight,
  PlayerId,
  Position,
  Width,
  Height,
  Direction,
  Active,
  Dead,
  Gravity,
  Animation,
  AnimationMax,
  Rotation,
  Location,
  ItemType,
  ItemScale,
  NetMode,
  Paused,
  Npcs,
  NpcLife,
  NpcActive,
  NpcFriendly,
  NpcImmortal,
  NpcInvulnerable,
  ItemKnockback,
  ItemDamage,ItemPick,ItemAxe,ItemHammer,ItemUseStyle,ItemUseAmmo,
  ItemMelee,ItemRanged,ItemMagic,ItemSummon,ItemConsumable,ItemAccessory,
  ItemNoUseGraphic,ItemChannel,ItemStaffSet,AmmoArrow,AmmoBullet,ItemAnimations,
  DrawPlayer,
  Shadow,
  MagicPixel,
  ItemTextures,
  World,
  DrawPosition,
  VisualOffset,
  DrawHidden,
  DrawHiddenExcept,
  Count
};
struct NativeRect {
  int x{}, y{}, w{}, h{};
};
struct NativeArm { bool enabled{}; unsigned char padding[3]{}; int stretch{}; float rotation{}; };
struct NativeHitSource {int item{},projectile{};Handle other{};};
static_assert(sizeof(NativeHitSource)==16);
static_assert(sizeof(NativeArm)==12);
struct BoundMethod {
  Handle metadata{};
  void *pointer{};
};
class RuntimeApi {
  std::array<Handle, static_cast<std::size_t>(Field::Count)> fields_{};
  struct InlineField {std::size_t offset{},size{};};
  std::array<InlineField,static_cast<std::size_t>(Field::Count)> inlineFields_{};
  const char *error_{};
  Handle assetValueField_{};

public:
  BoundMethod update, style, hitbox, hitNpc, heldDraw, heldItem, tick, menu,
      screen, arm, backArm, mountActive, solidCollision, hand, addDraw, setDraw, setDrawRect, mainUpdate, resetNpc;
  BoundMethod backDraw,weaponDamage,weaponKnockback,applyDamage,hitSource;
  bool wavesReady{};
  bool carryReady{};
  bool katanaInputReady{};
  ResolveReport resolve();
  Handle assetTexture(Handle asset) const;
  BoundMethod method(const char *ns, const char *type, const char *name,
                     bool instance, patch_type_t ret,
                     std::initializer_list<patch_type_t> args,
                     const char *structChoice = nullptr);
  bool field(Field, const char *, const char *, const char *, std::size_t);
  template <class T> T read(Field f, Handle obj = nullptr) const {
    T x{};
    auto index=static_cast<std::size_t>(f);
    auto h = fields_[index];
    const auto &inlineField=inlineFields_[index];
    if(h && inlineField.size) {
      if(obj && inlineField.size==sizeof(T))
        std::memcpy(&x,static_cast<const char *>(obj)+inlineField.offset,sizeof(T));
      return x;
    }
    if (h && patchlib_field_get_value)
      patchlib_field_get_value(h, obj, &x);
    return x;
  }
  template <class T> void write(Field f, Handle obj, T x) const {
    auto index=static_cast<std::size_t>(f);
    auto h = fields_[index];
    const auto &inlineField=inlineFields_[index];
    if(h && inlineField.size) {
      if(obj && inlineField.size==sizeof(T))
        std::memcpy(static_cast<char *>(obj)+inlineField.offset,&x,sizeof(T));
      return;
    }
    if (h && patchlib_field_set_value)
      patchlib_field_set_value(h, obj, &x);
  }
};
template <class R, class... A>
R invoke(const BoundMethod &m, Handle instance, A... args) {
  return reinterpret_cast<R (*)(Handle, A..., Handle)>(m.pointer)(
      instance, args..., m.metadata);
}
template <class R, class... A> R invokeStatic(const BoundMethod &m, A... args) {
  return reinterpret_cast<R (*)(A..., Handle)>(m.pointer)(args..., m.metadata);
}
} // namespace mr

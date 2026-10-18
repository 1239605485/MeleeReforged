#pragma once
#include "combat/attack_types.hpp"
#include "patchlib/field.h"
#include "patchlib/method.h"
#include "patchlib/struct/array.h"
#include <array>
#include <initializer_list>
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
struct BoundMethod {
  Handle metadata{};
  void *pointer{};
};
class RuntimeApi {
  std::array<Handle, static_cast<std::size_t>(Field::Count)> fields_{};
  const char *error_{};
  Handle assetValueField_{};

public:
  BoundMethod update, style, hitbox, hitNpc, heldDraw, heldItem, tick, menu,
      screen, arm, hand, addDraw, setDraw, mainUpdate, resetNpc;
  ResolveReport resolve();
  Handle assetTexture(Handle asset) const;
  BoundMethod method(const char *ns, const char *type, const char *name,
                     bool instance, patch_type_t ret,
                     std::initializer_list<patch_type_t> args,
                     const char *structChoice = nullptr);
  bool field(Field, const char *, const char *, const char *, std::size_t);
  template <class T> T read(Field f, Handle obj = nullptr) const {
    T x{};
    auto h = fields_[static_cast<std::size_t>(f)];
    if (h && patchlib_field_get_value)
      patchlib_field_get_value(h, obj, &x);
    return x;
  }
  template <class T> void write(Field f, Handle obj, T x) const {
    auto h = fields_[static_cast<std::size_t>(f)];
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

#include "runtime_api.hpp"
#include <cstring>
#include <dlfcn.h>
namespace mr {
static const char *argumentName(const char *method, std::size_t i) {
  const char *style[] = {"Single", "Item", "Rectangle"};
  const char *hitbox[] = {"Item", "Rectangle", "Boolean", "Rectangle"};
  const char *hit[] = {"Item", "Rectangle", "Int32", "Single", "Int32"};
  const char *draw[] = {"Texture2D", "Vector2", "Color",         "Single",
                        "Vector2",   "Vector2", "SpriteEffects", "Int32"};
  const char *arm[] = {"Boolean", "CompositeArmStretchAmount", "Single"};
  const char *hand[] = {"CompositeArmStretchAmount", "Single"};
  if (std::strcmp(method, "ItemCheck_ApplyUseStyle") == 0)
    return style[i];
  if (std::strcmp(method, "ItemCheck_GetMeleeHitbox") == 0)
    return hitbox[i];
  if (std::strcmp(method, "ProcessHitAgainstNPC") == 0)
    return hit[i];
  if (std::strcmp(method, "Set") == 0)
    return draw[i];
  if (std::strcmp(method, "SetCompositeArmFront") == 0)
    return arm[i];
  if (std::strcmp(method, "GetFrontHandPosition") == 0)
    return hand[i];
  if (std::strcmp(method, "DrawPlayer_27_HeldItem") == 0)
    return "PlayerDrawSet";
  if (std::strcmp(method, "Update") == 0 ||
      std::strcmp(method, "ResetNPCSlotData") == 0)
    return "Int32";
  if (std::strcmp(method, "DoUpdate") == 0)
    return "GameTime";
  return nullptr;
}
BoundMethod RuntimeApi::method(const char *ns, const char *type,
                               const char *name, bool instance,
                               patch_type_t ret,
                               std::initializer_list<patch_type_t> wanted,
                               const char *choice) {
  auto c = patchlib_type_get_type(ns, type);
  if (!c)
    return {};
  tefstd_vector_t methods{};
  if (!patchlib_type_get_methods(c, false, &methods))
    return {};
  BoundMethod found{};
  for (std::size_t i = 0; i < methods.size; ++i) {
    auto m = *static_cast<Handle *>(tefstd_vector_at(&methods, i));
    const char *n = patchlib_method_get_name(m);
    if (!n || std::strcmp(n, name) != 0 ||
        patchlib_method_get_param_count(m) != static_cast<int>(wanted.size()))
      continue;
    // The full structure fits the old prefix too. An old runtime leaves
    // arg_type_handles empty and is rejected.
    patch_method_signature_t s{};
    if (!patchlib_method_get_signature(m, &s))
      continue;
    bool ok = s.is_instance == instance && s.return_type == ret &&
              s.arg_types.size == wanted.size() &&
              s.arg_type_handles.size == wanted.size();
    std::size_t k = 0;
    for (auto t : wanted) {
      if (ok) {
        auto actual =
            static_cast<patch_type_t *>(tefstd_vector_at(&s.arg_types, k));
        ok = actual && *actual == t;
        auto expected = argumentName(name, k);
        auto h =
            static_cast<Handle *>(tefstd_vector_at(&s.arg_type_handles, k));
        auto actualName = h ? patchlib_type_get_name(*h) : nullptr;
        if (expected)
          ok = ok && actualName && std::strcmp(expected, actualName) == 0;
      }
      ++k;
    }
    if (ok && choice) {
      // DrawData.Set overloads differ at parameter 5 (float versus Vector2).
      auto h = static_cast<Handle *>(tefstd_vector_at(&s.arg_type_handles, 5));
      const char *n2 = h ? patchlib_type_get_name(*h) : nullptr;
      ok = n2 && std::strcmp(n2, choice) == 0;
    }
    patchlib_method_signature_free(&s);
    if (ok) {
      if (found.metadata) {
        found = {};
        break;
      }
      found = {m, patchlib_method_get_pointer(m)};
    }
  }
  tefstd_vector_destroy(&methods);
  return found;
}
bool RuntimeApi::field(Field slot, const char *ns, const char *type,
                       const char *name, std::size_t size) {
  auto c = patchlib_type_get_type(ns, type);
  auto f = c ? patchlib_type_get_field(c, name) : nullptr;
  if (!f || patchlib_field_get_size(f) != size) {
    error_ = name;
    return false;
  }
  fields_[static_cast<std::size_t>(slot)] = f;
  return true;
}
ResolveReport RuntimeApi::resolve() {
  if (!patchlib_type_get_type || !patchlib_type_get_methods ||
      !patchlib_method_get_name || !patchlib_method_get_param_count ||
      !patchlib_method_get_signature || !patchlib_method_signature_free ||
      !patchlib_method_get_pointer || !patchlib_type_get_field ||
      !patchlib_field_get_size || !patchlib_field_get_value ||
      !patchlib_field_set_value || !patchlib_type_get_name ||
      !patchlib_type_get_method_by_param_count || !tefstd_vector_at ||
      !tefstd_vector_destroy || !patchlib_array_at || !patchlib_array_length ||
      !patchlib_type_get_method_by_param_count)
    return {false, FailureCode::MissingExports, "TEFKernel exports"};
  update =
      method("Terraria", "Player", "Update", true, PATCH_VOID, {PATCH_INT32});
  mainUpdate =
      method("Terraria", "Main", "DoUpdate", true, PATCH_VOID, {PATCH_OBJECT});
  resetNpc = method("Terraria", "Player", "ResetNPCSlotData", false, PATCH_VOID,
                    {PATCH_INT32});
  style = method("Terraria", "Player", "ItemCheck_ApplyUseStyle", true,
                 PATCH_VOID, {PATCH_FLOAT, PATCH_OBJECT, PATCH_STRUCT});
  hitbox =
      method("Terraria", "Player", "ItemCheck_GetMeleeHitbox", true, PATCH_VOID,
             {PATCH_OBJECT, PATCH_STRUCT, PATCH_POINTER, PATCH_POINTER});
  hitNpc = method(
      "Terraria", "Player", "ProcessHitAgainstNPC", true, PATCH_VOID,
      {PATCH_OBJECT, PATCH_STRUCT, PATCH_INT32, PATCH_FLOAT, PATCH_INT32});
  heldDraw =
      method("Terraria.DataStructures", "PlayerDrawLayers",
             "DrawPlayer_27_HeldItem", false, PATCH_VOID, {PATCH_OBJECT});
  heldItem =
      method("Terraria", "Player", "get_HeldItem", true, PATCH_OBJECT, {});
  tick = method("Terraria", "Main", "get_GameUpdateCount", false, PATCH_UINT32,
                {});
  menu = method("Terraria", "Main", "get_gameMenu", false, PATCH_BOOL, {});
  screen =
      method("Terraria", "Main", "get_screenPosition", false, PATCH_STRUCT, {});
  arm = method("Terraria", "Player", "SetCompositeArmFront", true, PATCH_VOID,
               {PATCH_BOOL, PATCH_INT32, PATCH_FLOAT});
  hand = method("Terraria", "Player", "GetFrontHandPosition", true,
                PATCH_STRUCT, {PATCH_INT32, PATCH_FLOAT});
  addDraw = method("Terraria.DataStructures", "PlayerDrawSet", "AddDrawData",
                   true, PATCH_POINTER, {});
  setDraw =
      method("Terraria.DataStructures", "DrawData", "Set", true, PATCH_VOID,
             {PATCH_OBJECT, PATCH_STRUCT, PATCH_STRUCT, PATCH_FLOAT,
              PATCH_STRUCT, PATCH_STRUCT, PATCH_INT32, PATCH_INT32},
             "Vector2");
  BoundMethod all[] = {update,   style,   hitbox,  hitNpc,     heldDraw,
                       heldItem, tick,    menu,    screen,     arm,
                       hand,     addDraw, setDraw, mainUpdate, resetNpc};
  const char *names[] = {"Player.Update",        "ApplyUseStyle",
                         "GetMeleeHitbox",       "ProcessHitAgainstNPC",
                         "HeldItem draw",        "HeldItem getter",
                         "GameUpdateCount",      "gameMenu",
                         "screenPosition",       "SetCompositeArmFront",
                         "GetFrontHandPosition", "AddDrawData",
                         "DrawData.Set",         "Main.DoUpdate",
                         "ResetNPCSlotData"};
  for (std::size_t i = 0; i < sizeof(all) / sizeof(*all); ++i)
    if (!all[i].metadata || !all[i].pointer)
      return {false, FailureCode::SignatureMismatch, names[i]};
  bool ok = true;
  auto f = [&](Field s, const char *ns, const char *type, const char *n,
               std::size_t size) {
    if (ok)
      ok = field(s, ns, type, n, size);
  };
  f(Field::PlayerId, "Terraria", "Entity", "whoAmI", 4);
  f(Field::Position, "Terraria", "Entity", "position", 8);
  f(Field::Width, "Terraria", "Entity", "width", 4);
  f(Field::Height, "Terraria", "Entity", "height", 4);
  f(Field::Direction, "Terraria", "Entity", "direction", 4);
  f(Field::Active, "Terraria", "Player", "active", 1);
  f(Field::Dead, "Terraria", "Player", "dead", 1);
  f(Field::Gravity, "Terraria", "Player", "gravDir", 4);
  f(Field::Animation, "Terraria", "Player", "itemAnimation", 4);
  f(Field::AnimationMax, "Terraria", "Player", "itemAnimationMax", 4);
  f(Field::Rotation, "Terraria", "Player", "itemRotation", 4);
  f(Field::Location, "Terraria", "Player", "itemLocation", 8);
  f(Field::ItemType, "Terraria", "Item", "type", 4);
  f(Field::ItemScale, "Terraria", "Item", "scale", 4);
  f(Field::NetMode, "Terraria", "Main", "netMode", 4);
  f(Field::Paused, "Terraria", "Main", "gamePaused", 1);
  f(Field::Npcs, "Terraria", "Main", "npc", sizeof(void *));
  f(Field::NpcLife, "Terraria", "NPC", "life", 4);
  f(Field::NpcActive, "Terraria", "NPC", "active", 1);
  f(Field::DrawPlayer, "Terraria.DataStructures", "PlayerDrawSet", "drawPlayer",
    sizeof(void *));
  f(Field::Shadow, "Terraria.DataStructures", "PlayerDrawSet", "shadow", 4);
  f(Field::MagicPixel, "Terraria.GameContent", "TextureAssets", "MagicPixel",
    sizeof(void *));
  f(Field::ItemTextures, "Terraria.GameContent", "TextureAssets", "Item",
    sizeof(void *));
  f(Field::World, "Terraria", "Main", "ActiveWorldFileData", sizeof(void *));
  f(Field::DrawPosition, "Terraria.DataStructures", "PlayerDrawSet", "Position",
    8);
  f(Field::VisualOffset, "Terraria.DataStructures", "PlayerDrawSet",
    "VisualPositionOffset", 8);
  f(Field::DrawHidden, "Terraria.DataStructures", "PlayerDrawSet",
    "hideEntirePlayer", 1);
  f(Field::DrawHiddenExcept, "Terraria.DataStructures", "PlayerDrawSet",
    "hideEntirePlayerExceptHelmetsAndFaceAccessories", 1);
  if (!ok)
    return {false, FailureCode::MissingField, error_};
  // This Android Asset<T> exposes Value as a field. Its declared, closed
  // generic class comes from metadata, without assuming generic field offsets.
  Handle assetClass{};
  if (patchlib_field_get_type_handle) {
    assetClass = patchlib_field_get_type_handle(
        fields_[static_cast<std::size_t>(Field::MagicPixel)]);
  } else {
    // The supplied TPF table does not export field_get_type_handle. These two
    // exact IL2CPP APIs are also used by its source implementation.
    void *module = dlopen("libil2cpp.so", RTLD_NOW | RTLD_NOLOAD);
    if (!module)
      return {false, FailureCode::MissingExports, "libil2cpp metadata module"};
    using MetadataFn = Handle (*)(Handle);
    auto fieldType = reinterpret_cast<MetadataFn>(
        dlsym(module, "il2cpp_field_get_type"));
    auto classFromType = reinterpret_cast<MetadataFn>(
        dlsym(module, "il2cpp_class_from_type"));
    if (fieldType && classFromType)
      assetClass = classFromType(fieldType(
          fields_[static_cast<std::size_t>(Field::MagicPixel)]));
    dlclose(module);
  }
  if (!assetClass)
    return {false, FailureCode::MissingExports, "Asset<Texture2D> metadata"};
  assetValueField_ = patchlib_type_get_field(assetClass, "Value");
  if (!assetValueField_ ||
      patchlib_field_get_size(assetValueField_) != sizeof(Handle))
    return {false, FailureCode::MissingField, "Asset<Texture2D>.Value"};
  return {true, FailureCode::None, "ready"};
}
Handle RuntimeApi::assetTexture(Handle asset) const {
  Handle texture{};
  if (asset && assetValueField_)
    patchlib_field_get_value(assetValueField_, asset, &texture);
  return texture;
}
} // namespace mr

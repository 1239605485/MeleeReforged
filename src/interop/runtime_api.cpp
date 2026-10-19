#include "runtime_api.hpp"
#include <cstring>
#include <cstdio>
#include <dlfcn.h>
namespace mr {
void log(const char *);
namespace {
// Resolve raw metadata APIs rather than sharing patch_method_signature_t
// across independently versioned kernel and mod binaries.
struct Metadata {
  using Param = Handle (*)(Handle, unsigned);
  using Unary = Handle (*)(Handle);
  using Flag = bool (*)(Handle);
  using Kind = int (*)(Handle);
  using Name = const char *(*)(Handle);
  using ValueSize=std::int32_t (*)(Handle,std::uint32_t *);
  using Offset=std::size_t (*)(Handle);
  Param param{};
  Unary result{}, klass{};
  Flag instance{}, byref{}, enumeration{};
  Kind kind{};
  Name name{};
  Unary fieldType{};
  ValueSize valueSize{};
  Offset fieldOffset{};
  Metadata() {
    void *lib=dlopen("libil2cpp.so", RTLD_NOW | RTLD_NOLOAD);
    if (!lib) return;
#define META(slot, symbol) slot=reinterpret_cast<decltype(slot)>(dlsym(lib, symbol))
    META(param,"il2cpp_method_get_param");
    META(result,"il2cpp_method_get_return_type");
    META(klass,"il2cpp_class_from_type");
    META(instance,"il2cpp_method_is_instance");
    META(byref,"il2cpp_type_is_byref");
    META(enumeration,"il2cpp_class_is_enum");
    META(kind,"il2cpp_type_get_type");
    META(name,"il2cpp_class_get_name");
    META(fieldType,"il2cpp_field_get_type");
    META(valueSize,"il2cpp_class_value_size");
    META(fieldOffset,"il2cpp_field_get_offset");
#undef META
    dlclose(lib);
  }
  bool ready() const {
    return param && result && klass && instance && byref && enumeration && kind && name;
  }
  patch_type_t category(Handle type) const {
    if (!type) return PATCH_VOID;
    if (byref(type)) return PATCH_POINTER;
    switch(kind(type)) {
      case 1: return PATCH_VOID;
      case 2: return PATCH_BOOL;
      case 3: return PATCH_CHAR;
      case 4: return PATCH_INT8;
      case 5: return PATCH_UINT8;
      case 6: return PATCH_INT16;
      case 7: return PATCH_UINT16;
      case 8: return PATCH_INT32;
      case 9: return PATCH_UINT32;
      case 10: return PATCH_INT64;
      case 11: return PATCH_UINT64;
      case 12: return PATCH_FLOAT;
      case 13: return PATCH_DOUBLE;
      case 15: return PATCH_POINTER;
      case 17: {
        auto typeName=name(klass(type));
        if(typeName && (std::strcmp(typeName,"IntPtr")==0 || std::strcmp(typeName,"UIntPtr")==0))
          return PATCH_POINTER;
        return enumeration(klass(type)) ? PATCH_INT32 : PATCH_STRUCT;
      }
      case 18: return PATCH_OBJECT;
      case 24: case 25: return PATCH_POINTER; // IL2CPP_TYPE_I / IL2CPP_TYPE_U.
      default: return PATCH_OBJECT;
    }
  }
  const char *typeName(Handle type) const {
    auto c=type ? klass(type) : nullptr;
    return c ? name(c) : nullptr;
  }
};
}
static const char *argumentName(const char *method, std::size_t i, std::size_t count) {
  const char *style[] = {"Single", "Item", "Rectangle"};
  const char *hitbox[] = {"Item", "Rectangle", "Boolean", "Rectangle"};
  const char *hit[] = {"Item", "Rectangle", "Int32", "Single", "Int32"};
  const char *draw[] = {"Texture2D", "Vector2", "Color",         "Single",
                        "Vector2",   "Vector2", "SpriteEffects", "Int32"};
  const char *drawRect[] = {"Texture2D", "Vector2", "Rectangle", "Color", "Single", "Vector2", "Vector2", "SpriteEffects", "Int32"};
  const char *arm[] = {"Boolean", "CompositeArmStretchAmount", "Single"};
  const char *hand[] = {"CompositeArmStretchAmount", "Single"};
  if (std::strcmp(method, "ItemCheck_ApplyUseStyle") == 0)
    return style[i];
  if (std::strcmp(method, "ItemCheck_GetMeleeHitbox") == 0)
    return hitbox[i];
  if (std::strcmp(method, "ProcessHitAgainstNPC") == 0)
    return hit[i];
  if (std::strcmp(method, "Set") == 0)
    return count==9 ? drawRect[i] : draw[i];
  if (std::strcmp(method, "SetCompositeArmFront") == 0 || std::strcmp(method,"SetCompositeArmBack")==0)
    return arm[i];
  if (std::strcmp(method, "GetFrontHandPosition") == 0)
    return hand[i];
  if (std::strcmp(method, "DrawPlayer_27_HeldItem") == 0)
    return "PlayerDrawSet";
  if(std::strcmp(method,"DrawPlayer_08_Backpacks")==0)return "PlayerDrawSet";
  if(std::strcmp(method,"GetWeaponDamage")==0)return "Item";
  if(std::strcmp(method,"GetWeaponKnockback")==0)return i==0?"Item":"Single";
  if(std::strcmp(method,"ApplyDamageToNPC")==0){
    const char *args[]={"NPC","Int32","Single","Int32","Boolean","PlayerNPCHitSource"};return args[i];
  }
  if(std::strcmp(method,"FromItem")==0)return "Int32";
  if(std::strcmp(method,"GetFrame")==0)return i==0?"Texture2D":"Int32";
  if (std::strcmp(method, "Update") == 0 ||
      std::strcmp(method, "ResetNPCSlotData") == 0)
    return "Int32";
  if (std::strcmp(method, "DoUpdate") == 0)
    return "GameTime";
  if(std::strcmp(method,".ctor")==0 && count==1) return "Texture2D";
  if(std::strcmp(method,"SolidCollision")==0) {
    const char *args[]={"Vector2","Int32","Int32","Boolean"};return args[i];
  }
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
    Metadata raw;
    if (!raw.ready()) {
      log("Method binding failed: required libil2cpp metadata exports unavailable.");
      break;
    }
    bool ok = raw.result(m) && raw.instance(m) == instance && raw.category(raw.result(m)) == ret;
    std::size_t k = 0;
    for (auto t : wanted) {
      auto parameter=raw.param(m, static_cast<unsigned>(k));
      auto actualName=raw.typeName(parameter);
      auto expected=argumentName(name,k,wanted.size());
      bool matches=parameter && raw.category(parameter)==t &&
          (!expected || (actualName && std::strcmp(expected,actualName)==0));
      if (!matches) {
        char message[256];
        std::snprintf(message,sizeof(message),"%s.%s parameter %zu mismatch: type=%s category=%d expected=%d",
          type,name,k,actualName ? actualName : "null",int(raw.category(parameter)),int(t));
        log(message);
      }
      ok=ok && matches;
      ++k;
    }
    if (ok && choice) {
      auto n2=raw.typeName(raw.param(m,5));
      ok=n2 && std::strcmp(n2,choice)==0;
    }
    if (!ok) {
      char message[256];
      std::snprintf(message,sizeof(message),"%s.%s rejected: instance=%d expected=%d return=%d expected=%d",
          type,name,raw.instance(m),instance,int(raw.category(raw.result(m))),int(ret));
      log(message);
    }
    if (ok) {
      if (found.metadata) {
        found = {};
        break;
      }
      found = {m, patchlib_method_get_pointer(m)};
    }
  }
  tefstd_vector_destroy(&methods);
  if (!found.metadata || !found.pointer) {
    char message[192];
    std::snprintf(message,sizeof(message),"Method unavailable or ambiguous: %s.%s (%zu parameters)",type,name,wanted.size());
    log(message);
  }
  return found;
}
bool RuntimeApi::field(Field slot, const char *ns, const char *type,
                       const char *name, std::size_t size) {
  auto index=static_cast<std::size_t>(slot);
  fields_[index]=nullptr;inlineFields_[index]={};
  auto c = patchlib_type_get_type(ns, type);
  auto f = c ? patchlib_type_get_field(c, name) : nullptr;
  // The supplied Android kernel's VALUETYPE size branch falls through to
  // sizeof(void*). Its get/set helpers also use that truncated size. Resolve
  // these reference-free inline structs from IL2CPP metadata and copy their
  // verified full payload; changing the expected size to 8 would lose rotation.
  if(f && (slot==Field::FrontArm || slot==Field::BackArm)) {
    Metadata raw;
    if(raw.ready() && raw.fieldType && raw.valueSize && raw.fieldOffset) {
      auto fieldType=raw.fieldType(f);
      auto klass=fieldType?raw.klass(fieldType):nullptr;
      auto actualName=klass?raw.name(klass):nullptr;
      auto actualSize=klass?raw.valueSize(klass,nullptr):0;
      auto offset=raw.fieldOffset(f);
      if(fieldType && raw.kind(fieldType)==17 && actualName &&
         std::strcmp(actualName,"CompositeArmData")==0 &&
         actualSize>0 && static_cast<std::size_t>(actualSize)==size &&
         offset>0 && offset<1024*1024) {
        fields_[index]=f;inlineFields_[index]={offset,size};
        return true;
      }
      char message[192];
      std::snprintf(message,sizeof(message),"Inline field %s layout rejected: actual=%d expected=%zu",name,actualSize,size);
      log(message);
    }
    error_=name;
    return false;
  }
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
  backDraw=method("Terraria.DataStructures","PlayerDrawLayers","DrawPlayer_08_Backpacks",false,PATCH_VOID,{PATCH_OBJECT});
  weaponDamage=method("Terraria","Player","GetWeaponDamage",true,PATCH_INT32,{PATCH_OBJECT});
  weaponKnockback=method("Terraria","Player","GetWeaponKnockback",true,PATCH_FLOAT,{PATCH_OBJECT,PATCH_FLOAT});
  applyDamage=method("Terraria","Player","ApplyDamageToNPC",true,PATCH_VOID,
       {PATCH_OBJECT,PATCH_INT32,PATCH_FLOAT,PATCH_INT32,PATCH_BOOL,PATCH_STRUCT});
  hitSource=method("Terraria.GameContent","PlayerNPCHitSource","FromItem",false,PATCH_STRUCT,{PATCH_INT32});
  wavesReady=weaponDamage.pointer && weaponKnockback.pointer && applyDamage.pointer && hitSource.pointer;
  heldItem =
      method("Terraria", "Player", "get_HeldItem", true, PATCH_OBJECT, {});
  tick = method("Terraria", "Main", "get_GameUpdateCount", false, PATCH_UINT32,
                {});
  menu = method("Terraria", "Main", "get_gameMenu", false, PATCH_BOOL, {});
  screen =
      method("Terraria", "Main", "get_screenPosition", false, PATCH_STRUCT, {});
  arm = method("Terraria", "Player", "SetCompositeArmFront", true, PATCH_VOID,
               {PATCH_BOOL, PATCH_INT32, PATCH_FLOAT});
  backArm = method("Terraria", "Player", "SetCompositeArmBack", true, PATCH_VOID,
               {PATCH_BOOL, PATCH_INT32, PATCH_FLOAT});
  mountActive=method("Terraria","Mount","get_Active",true,PATCH_BOOL,{});
  solidCollision=method("Terraria","Collision","SolidCollision",false,PATCH_BOOL,
                        {PATCH_STRUCT,PATCH_INT32,PATCH_INT32,PATCH_BOOL});
  hand = method("Terraria", "Player", "GetFrontHandPosition", true,
                PATCH_STRUCT, {PATCH_INT32, PATCH_FLOAT});
  addDraw = method("Terraria.DataStructures", "PlayerDrawSet", "AddDrawData",
                   true, PATCH_POINTER, {});
  setDraw =
      method("Terraria.DataStructures", "DrawData", "Set", true, PATCH_VOID,
             {PATCH_OBJECT, PATCH_STRUCT, PATCH_STRUCT, PATCH_FLOAT,
              PATCH_STRUCT, PATCH_STRUCT, PATCH_INT32, PATCH_INT32},
             "Vector2");
  setDrawRect = method("Terraria.DataStructures", "DrawData", "Set", true, PATCH_VOID,
      {PATCH_OBJECT,PATCH_STRUCT,PATCH_STRUCT,PATCH_STRUCT,PATCH_FLOAT,
       PATCH_STRUCT,PATCH_STRUCT,PATCH_INT32,PATCH_INT32});
  BoundMethod all[] = {update,   style,   hitbox,  hitNpc,     heldDraw,
                       heldItem, tick,    menu,    screen,     arm,
                       hand,     addDraw, setDraw, mainUpdate, resetNpc, setDrawRect, backArm, mountActive, solidCollision};
  const char *names[] = {"Player.Update",        "ApplyUseStyle",
                         "GetMeleeHitbox",       "ProcessHitAgainstNPC",
                         "HeldItem draw",        "HeldItem getter",
                         "GameUpdateCount",      "gameMenu",
                         "screenPosition",       "SetCompositeArmFront",
                         "GetFrontHandPosition", "AddDrawData",
                         "DrawData.Set",         "Main.DoUpdate",
                         "ResetNPCSlotData", "DrawData.Set source rectangle", "SetCompositeArmBack", "Mount.Active", "Collision.SolidCollision"};
  for (std::size_t i = 0; i < sizeof(all) / sizeof(*all); ++i)
    if (!all[i].metadata || !all[i].pointer)
      return {false, FailureCode::SignatureMismatch, names[i]};
  if(!backDraw.pointer)return {false,FailureCode::SignatureMismatch,"Backpack draw layer"};
  bool ok = true;
  auto f = [&](Field s, const char *ns, const char *type, const char *n,
               std::size_t size) {
    if (ok)
      ok = field(s, ns, type, n, size);
  };
  f(Field::ControlUseItem,"Terraria","Player","controlUseItem",1);
  f(Field::BodyRotation,"Terraria","Player","bodyRotation",4);
  f(Field::BodyPosition,"Terraria","Player","bodyPosition",8);
  f(Field::LegRotation,"Terraria","Player","legRotation",4);
  f(Field::LegPosition,"Terraria","Player","legPosition",8);
  f(Field::FrontArm,"Terraria","Player","compositeFrontArm",12);
  f(Field::BackArm,"Terraria","Player","compositeBackArm",12);
  f(Field::Velocity,"Terraria","Entity","velocity",8);
  f(Field::Mount,"Terraria","Player","mount",sizeof(Handle));
  f(Field::Pulley,"Terraria","Player","pulley",1);
  f(Field::GrapCount,"Terraria","Player","grapCount",4);
  f(Field::Frozen,"Terraria","Player","frozen",1);
  f(Field::Stoned,"Terraria","Player","stoned",1);
  f(Field::TextureWidth,"Microsoft.Xna.Framework.Graphics","Texture2D","Width",4);
  f(Field::TextureHeight,"Microsoft.Xna.Framework.Graphics","Texture2D","Height",4);
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
  f(Field::NpcFriendly,"Terraria","NPC","friendly",1);
  f(Field::NpcImmortal,"Terraria","NPC","immortal",1);
  f(Field::NpcInvulnerable,"Terraria","NPC","dontTakeDamage",1);
  f(Field::ItemKnockback,"Terraria","Item","knockBack",4);
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
  if(!wavesReady)log("Sword wave disabled: damage bindings unavailable; melee remains active.");
  carryReady=true;
  auto cf=[&](Field slot,const char *ns,const char *type,const char *name,std::size_t size){
    if(!field(slot,ns,type,name,size))carryReady=false;
  };
  cf(Field::ItemDamage,"Terraria","Item","damage",4);
  cf(Field::ItemPick,"Terraria","Item","pick",4);
  cf(Field::ItemAxe,"Terraria","Item","axe",4);
  cf(Field::ItemHammer,"Terraria","Item","hammer",4);
  cf(Field::ItemUseStyle,"Terraria","Item","useStyle",1);
  cf(Field::ItemUseAmmo,"Terraria","Item","useAmmo",4);
  cf(Field::ItemMelee,"Terraria","Item","melee",1);
  cf(Field::ItemRanged,"Terraria","Item","ranged",1);
  cf(Field::ItemMagic,"Terraria","Item","magic",1);
  cf(Field::ItemSummon,"Terraria","Item","summon",1);
  cf(Field::ItemConsumable,"Terraria","Item","consumable",1);
  cf(Field::ItemAccessory,"Terraria","Item","accessory",1);
  cf(Field::ItemNoUseGraphic,"Terraria","Item","noUseGraphic",1);
  cf(Field::ItemChannel,"Terraria","Item","channel",1);
  cf(Field::ItemStaffSet,"Terraria","Item","staff",sizeof(Handle));
  cf(Field::AmmoArrow,"Terraria.ID","AmmoID","Arrow",4);
  cf(Field::AmmoBullet,"Terraria.ID","AmmoID","Bullet",4);
  cf(Field::ItemAnimations,"Terraria","Main","itemAnimations",sizeof(Handle));
  if(!carryReady)log("Generic carry disabled: item metadata unavailable; wooden sword remains active.");
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

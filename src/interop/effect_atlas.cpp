#include "effect_atlas.hpp"
#include "generated/slash_atlas.hpp"
#include <cstring>
#include <dlfcn.h>
namespace mr {
void log(const char *);
Handle effectAtlas(RuntimeApi &api) {
  static bool attempted=false;
  static Handle atlas{};
  static unsigned root{};
  if(attempted) return atlas;
  attempted=true;
  auto ctor=api.method("UnityEngine","Texture2D",".ctor",true,PATCH_VOID,
                       {PATCH_INT32,PATCH_INT32,PATCH_INT32,PATCH_BOOL});
  auto raw=api.method("UnityEngine","Texture2D","GetWritableImageData",true,PATCH_POINTER,{PATCH_INT32});
  auto size=api.method("UnityEngine","Texture2D","GetRawImageDataSize",true,PATCH_UINT64,{});
  auto apply=api.method("UnityEngine","Texture2D","Apply",true,PATCH_VOID,{});
  auto filter=api.method("UnityEngine","Texture","set_filterMode",true,PATCH_VOID,{PATCH_INT32});
  auto wrap=api.method("UnityEngine","Texture","set_wrapMode",true,PATCH_VOID,{PATCH_INT32});
  auto xnaCtor=api.method("Microsoft.Xna.Framework.Graphics","Texture2D",".ctor",true,PATCH_VOID,{PATCH_OBJECT});
  if(!ctor.pointer || !raw.pointer || !size.pointer || !apply.pointer ||
     !filter.pointer || !wrap.pointer || !xnaCtor.pointer) {
    log("Slash atlas disabled: texture metadata unavailable; geometry fallback active.");return nullptr;
  }
  void *module=dlopen("libil2cpp.so",RTLD_NOW|RTLD_NOLOAD);
  if(!module) return nullptr;
  auto make=reinterpret_cast<Handle (*)(Handle)>(dlsym(module,"il2cpp_object_new"));
  auto retain=reinterpret_cast<unsigned (*)(Handle,bool)>(dlsym(module,"il2cpp_gchandle_new"));
  auto release=reinterpret_cast<void (*)(unsigned)>(dlsym(module,"il2cpp_gchandle_free"));
  dlclose(module);
  if(!make || !retain || !release) return nullptr;
  auto unity=make(patchlib_type_get_type("UnityEngine","Texture2D"));
  if(!unity) return nullptr;
  unsigned unityRoot=retain(unity,false);
  if(!unityRoot) return nullptr;
  invoke<void>(ctor,unity,slashAtlasWidth,slashAtlasHeight,4,false);
  invoke<void>(filter,unity,1); // Bilinear; texture contains a soft halo row.
  invoke<void>(wrap,unity,1);   // Clamp.
  auto pixels=invoke<void *>(raw,unity,0);
  if(!pixels || invoke<std::uint64_t>(size,unity)!=sizeof(slashAtlasPixels)) {
    release(unityRoot);log("Slash atlas raw size mismatch; geometry fallback active.");return nullptr;
  }
  std::memcpy(pixels,slashAtlasPixels,sizeof(slashAtlasPixels));
  invoke<void>(apply,unity);
  auto xna=make(patchlib_type_get_type("Microsoft.Xna.Framework.Graphics","Texture2D"));
  if(!xna) {release(unityRoot);return nullptr;}
  root=retain(xna,false);
  if(!root) {release(unityRoot);return nullptr;}
  invoke<void>(xnaCtor,xna,unity);
  release(unityRoot); // XNA owns the Unity texture reference from here.
  atlas=xna;
  log("Slash atlas ready: six textured frames, core/halo, GC retained.");
  return atlas;
}
}

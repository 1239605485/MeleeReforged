#include "item_visuals.hpp"
#include <dlfcn.h>
namespace mr {
ItemVisual itemVisual(RuntimeApi &api,int id){
 auto items=api.read<Handle>(Field::ItemTextures);Handle asset{};
 if(id<=0 || !items || static_cast<std::size_t>(id)>=patchlib_array_length(items) || !patchlib_array_at(items,id,&asset))return {};
 auto texture=api.assetTexture(asset);if(!texture)return {};
 int width=api.read<int>(Field::TextureWidth,texture),height=api.read<int>(Field::TextureHeight,texture);
 if(width<=0 || height<=0 || width>4096 || height>4096)return {};
 NativeRect frame{0,0,width,height};
 auto animations=api.read<Handle>(Field::ItemAnimations);Handle animation{};
 if(animations && static_cast<std::size_t>(id)<patchlib_array_length(animations) && patchlib_array_at(animations,id,&animation) && animation){
  struct Frames {
   using ObjectClass=Handle (*)(Handle);ObjectClass objectClass{};
   Handle classes[3]{};BoundMethod methods[3]{};
   explicit Frames(RuntimeApi &a){
    auto module=dlopen("libil2cpp.so",RTLD_NOW|RTLD_NOLOAD);
    if(module){objectClass=reinterpret_cast<ObjectClass>(dlsym(module,"il2cpp_object_get_class"));dlclose(module);}
    const char *names[]={"DrawAnimation","DrawAnimationVertical","DrawAnimationScryingOrb"};
    for(int i=0;i<3;++i){
     classes[i]=patchlib_type_get_type("Terraria.DataStructures",names[i]);
     if(classes[i])methods[i]=a.method("Terraria.DataStructures",names[i],"GetFrame",true,PATCH_STRUCT,{PATCH_OBJECT,PATCH_INT32});
    }
   }
  };
  static Frames frames(api);
  if(!frames.objectClass)return {};
  auto klass=frames.objectClass(animation);bool found=false;
  for(int i=0;i<3;++i)if(klass==frames.classes[i] && frames.methods[i].pointer){
   frame=invoke<NativeRect>(frames.methods[i],animation,texture,-1);found=true;break;
  }
  if(!found)return {}; // Unknown animation types stay on vanilla rendering.
 }
 if(frame.x<0 || frame.y<0 || frame.w<=0 || frame.h<=0 || frame.x+frame.w>width || frame.y+frame.h>height)return {};
 return {texture,frame};
}
}

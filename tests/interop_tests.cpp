#include "interop/hook_set.hpp"
#include "interop/runtime_api.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      std::fprintf(stderr, "line %d: %s\n", __LINE__, #x);                     \
      std::exit(1);                                                            \
    }                                                                          \
  } while (0)
namespace mr { void log(const char *) {} }
static int installs, removed;
static int install(void *, void *, void **original) {
  *original = reinterpret_cast<void *>(9);
  return ++installs == 3 ? -1 : 0;
}
static int uninstall(void *h) {
  removed += static_cast<int>(reinterpret_cast<std::uintptr_t>(h));
  return 0;
}
static int failRemove(void *) { return -1; }
static void *metadata = reinterpret_cast<void *>(1);
static void *getType(const char *, const char *) { return metadata; }
static bool getMethods(void *, bool, tefstd_vector_t *v) {
  *v = {&metadata, 1, 1, sizeof(void *)}; return true;
}
static const char *methodName="Update";
static const char *getName(void *) { return methodName; }
static int getCount(void *) { return 1; }
static void *at(const tefstd_vector_t *v, std::size_t i) {
  return i < v->size ? static_cast<char *>(v->data)+i*v->elem_size : nullptr;
}
static void destroy(tefstd_vector_t *) {}
static void *pointer(void *) { return reinterpret_cast<void *>(9); }
// Simulates the older runtime: the extended signature tail is absent.
static bool oldSignature(void *, patch_method_signature_t *s) {
  static patch_type_t arg = PATCH_INT32;
  s->is_instance=true; s->return_type=PATCH_VOID;
  s->arg_types={&arg,1,1,sizeof(arg)}; return true;
}
static void *getField(void *,const char *) {return metadata;}
static std::size_t brokenFieldSize(void *) {return 8;}
static int truncatedReads,truncatedWrites;
static void brokenRead(void *,void *obj,void *value) {++truncatedReads;std::memcpy(value,static_cast<char *>(obj)+8,8);}
static void brokenWrite(void *,void *obj,void *value) {++truncatedWrites;std::memcpy(static_cast<char *>(obj)+8,value,8);}
static bool freeSignature(patch_method_signature_t *) { return true; }
static void resolverChecks(mr::RuntimeApi &api) {
  void *lib=dlopen(METADATA_FIXTURE, RTLD_NOW | RTLD_GLOBAL);
  CHECK(lib);
  patchlib_type_get_type=getType; patchlib_type_get_methods=getMethods;
  patchlib_method_get_name=getName; patchlib_method_get_param_count=getCount;
  patchlib_method_get_signature=oldSignature;
  patchlib_method_signature_free=freeSignature;
  patchlib_method_get_pointer=pointer;
  tefstd_vector_at=at; tefstd_vector_destroy=destroy;
  auto bind=[&] { return api.method("Terraria","Player","Update",true,PATCH_VOID,{PATCH_INT32}); };
  CHECK(bind().metadata == metadata);
  auto set=reinterpret_cast<void (*)(int,const char *,bool)>(dlsym(lib,"fixture_type"));
  CHECK(set);
  set(12,"Single",false); CHECK(!bind().metadata);
  set(8,"Int32",true); CHECK(!bind().metadata);
  set(8,"WrongType",false); CHECK(!bind().metadata);
  set(8,"Int32",false); CHECK(bind().pointer == pointer(nullptr));
  auto result=reinterpret_cast<void (*)(int,const char *)>(dlsym(lib,"fixture_result"));CHECK(result);
  methodName="GetWritableImageData";
  result(24,"IntPtr");
  CHECK(api.method("UnityEngine","Texture2D",methodName,true,PATCH_POINTER,{PATCH_INT32}).metadata);
  result(17,"IntPtr");
  CHECK(api.method("UnityEngine","Texture2D",methodName,true,PATCH_POINTER,{PATCH_INT32}).metadata);
  result(1,"Void");methodName="Update";
  // Reproduce the Android kernel's VALUETYPE fall-through: every struct reports 8.
  patchlib_type_get_field=getField;patchlib_field_get_size=brokenFieldSize;
  patchlib_field_get_value=brokenRead;patchlib_field_set_value=brokenWrite;
  CHECK(api.field(mr::Field::FrontArm,"Terraria","Player","compositeFrontArm",12));
  CHECK(api.field(mr::Field::BackArm,"Terraria","Player","compositeBackArm",12));
  struct Object {std::uint64_t prefix;mr::NativeArm arm;std::uint32_t suffix;};
  Object obj{0x12345678,{true,{0,0,0},3,1.25f},0xabcdef01};
  CHECK(offsetof(Object,arm)==8);
  auto arm=api.read<mr::NativeArm>(mr::Field::FrontArm,&obj);
  CHECK(arm.enabled && arm.stretch==3 && arm.rotation==1.25f);
  mr::NativeArm replacement{false,{0,0,0},0,-2.5f};
  api.write(mr::Field::BackArm,&obj,replacement);
  CHECK(obj.arm.rotation==-2.5f && obj.arm.stretch==0 && !obj.arm.enabled);
  CHECK(obj.prefix==0x12345678 && obj.suffix==0xabcdef01);
  CHECK(truncatedReads==0 && truncatedWrites==0);
  // A genuine layout mismatch still fails, rather than disabling validation.
  auto armSize=reinterpret_cast<void (*)(int)>(dlsym(lib,"fixture_arm_size"));CHECK(armSize);
  armSize(16);CHECK(!api.field(mr::Field::FrontArm,"Terraria","Player","compositeFrontArm",12));
  armSize(12);
  dlclose(lib);
}
int main() {
  mr::RuntimeApi api;
  CHECK(!api.resolve().compatible);
  resolverChecks(api);
  void *original{};
  mr::HookSet hooks(install, uninstall);
  mr::HookRequest r{reinterpret_cast<void *>(1), reinterpret_cast<void *>(8),
                    &original};
  CHECK(hooks.install(r));
  r.target = reinterpret_cast<void *>(2);
  CHECK(hooks.install(r));
  r.target = reinterpret_cast<void *>(3);
  CHECK(!hooks.install(r));
  CHECK(removed == 3);
  CHECK(hooks.reset());
  CHECK(removed == 3);
  installs = 0;
  mr::HookSet broken(install, failRemove);
  CHECK(broken.install(r));
  CHECK(!broken.reset());
  CHECK(broken.count() == 1);
  CHECK(!broken.install(r));
  std::puts("interop checks passed");
}

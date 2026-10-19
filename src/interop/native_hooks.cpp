#include "native_hooks.hpp"
#include "dobby.h"
#include "game_runtime.hpp"
#include <array>
namespace mr {
namespace {
using Update = void (*)(Handle, int, Handle);
using Style = void (*)(Handle, float, Handle, NativeRect, Handle);
using Hitbox = void (*)(Handle, Handle, NativeRect, bool *, NativeRect *,
                        Handle);
using Hit = void (*)(Handle, Handle, NativeRect, int, float, int, Handle);
using Draw = void (*)(Handle, Handle);
using MainUpdate = void (*)(Handle, Handle, Handle);
using ResetNpc = void (*)(int, Handle);
Update originalUpdate{};
Style originalStyle{};
Hitbox originalHitbox{};
Hit originalHit{};
Draw originalDraw{};
Draw originalBackDraw{};
MainUpdate originalMain{};
ResetNpc originalReset{};
HookSet hooks(DobbyHook, DobbyDestroy);
void mainHook(Handle p, Handle time, Handle metadata) {
  runtime().frame();
  originalMain(p, time, metadata);
  runtime().frame();
}
void resetHook(int id, Handle metadata) {
  originalReset(id, metadata);
  runtime().resetNpc(id);
}
void updateHook(Handle p, int i, Handle metadata) {
  runtime().restorePose(p);
  originalUpdate(p, i, metadata);
  runtime().update(p);
}
void styleHook(Handle p, float offset, Handle item, NativeRect frame,
               Handle metadata) {
  originalStyle(p, offset, item, frame, metadata);
  runtime().action(p);
}
void hitboxHook(Handle p, Handle item, NativeRect frame, bool *dont,
                NativeRect *rectangle, Handle metadata) {
  void *args[] = {&item, &frame, &dont, &rectangle};
  if (!runtime().hitbox(p, args))
    originalHitbox(p, item, frame, dont, rectangle, metadata);
}
void hitHook(Handle p, Handle item, NativeRect rectangle, int damage,
             float knockback, int id, Handle metadata) {
  void *args[] = {&item, &rectangle, &damage, &knockback, &id};
  if (!runtime().hitNpc(p, args))
    originalHit(p, item, rectangle, damage, knockback, id, metadata);
  runtime().afterHit(p, args);
}
void drawHook(Handle info, Handle metadata) {
  if (!runtime().draw(info))
    originalDraw(info, metadata);
}
void backDrawHook(Handle info,Handle metadata){
  originalBackDraw(info,metadata);
  runtime().drawBack(info);
}
} // namespace
bool installNativeHooks(RuntimeApi &a) {
  if (hooks.count())
    return !hooks.failed();
  if (hooks.failed())
    return false;
  struct Request {
    void *target;
    void *callback;
    void **original;
  };
  Request requests[] = {
      {a.update.pointer, reinterpret_cast<void *>(updateHook),
       reinterpret_cast<void **>(&originalUpdate)},
      {a.style.pointer, reinterpret_cast<void *>(styleHook),
       reinterpret_cast<void **>(&originalStyle)},
      {a.hitbox.pointer, reinterpret_cast<void *>(hitboxHook),
       reinterpret_cast<void **>(&originalHitbox)},
      {a.hitNpc.pointer, reinterpret_cast<void *>(hitHook),
       reinterpret_cast<void **>(&originalHit)},
      {a.heldDraw.pointer, reinterpret_cast<void *>(drawHook),
       reinterpret_cast<void **>(&originalDraw)},
      {a.backDraw.pointer,reinterpret_cast<void *>(backDrawHook),
       reinterpret_cast<void **>(&originalBackDraw)},
      {a.mainUpdate.pointer, reinterpret_cast<void *>(mainHook),
       reinterpret_cast<void **>(&originalMain)},
      {a.resetNpc.pointer, reinterpret_cast<void *>(resetHook),
       reinterpret_cast<void **>(&originalReset)}};
  for (auto &r : requests) {
    if (!hooks.install({r.target, r.callback, r.original}))
      return false;
  }
  return true;
}
bool removeNativeHooks() {
  bool ok = hooks.reset();
  if (!ok)
    log("Native hook removal failed; process restart is required.");
  return ok;
}
} // namespace mr

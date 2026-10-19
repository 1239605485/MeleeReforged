#include "interop/draw_bridge.hpp"
#include <cstdio>
#include <cstdlib>
#define CHECK(x) do { if(!(x)) {std::fprintf(stderr,"line %d: %s\n",__LINE__,#x);std::exit(1);} } while(0)
using namespace mr;
static char storage[128]; static bool drawn;
static void *add(void *ctx,void *meta) {CHECK(ctx==(void*)1 && meta==(void*)3);return storage;}
static void set(void *slot,void *texture,Vec2 pos,NativeRect rect,NativeColor c,float angle,Vec2 origin,Vec2 scale,int effect,int depth,void *meta) {
 CHECK(slot==storage && texture==(void*)2 && meta==(void*)4);
 CHECK(pos.x==10 && pos.y==20 && rect.x==0 && rect.y==0 && rect.w==1 && rect.h==1);
 CHECK(c.a==128 && c.b==110 && c.g==50 && c.r==10);
 CHECK(angle==.5f && origin.x==0 && origin.y==.5f && scale.x==31 && scale.y==4);
 CHECK(effect==0 && depth==0);drawn=true;
}
int main() {
 RuntimeApi api;api.addDraw={(void*)3,reinterpret_cast<void*>(add)};
 api.setDrawRect={(void*)4,reinterpret_cast<void*>(set)};
 CHECK(enqueueDraw(api,(void*)1,(void*)2,{10,20},{20,100,220,128},.5f,{0,.5f},{31,4},true));
 CHECK(drawn);
 CHECK(!enqueueDraw(api,(void*)1,nullptr,{},{},0,{},{},true));
 std::puts("render bridge checks passed");
}

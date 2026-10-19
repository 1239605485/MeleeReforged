#include "render/katana_trail.hpp"
#include "combat/trajectory.hpp"
#include "weapons/weapon_registry.hpp"
#include <cassert>
#include <cmath>
int main(){
 auto &w=mr::katana();mr::TrailBuffer history;
 auto a=mr::evaluatePose(w,0,.19f,{},1,1,34,false);
 auto b=mr::evaluatePose(w,0,.43f,{},1,1,34,false);
 a.tick=10;b.tick=11;a.attackId=b.attackId=1;
 auto ribbon=mr::katanaTrail(a,b,history);assert(ribbon.count>2 && ribbon.count<=32);
 auto am=mr::evaluatePose(w,0,.19f,{},-1,-1,34,false);
 auto bm=mr::evaluatePose(w,0,.43f,{},-1,-1,34,false);
 am.tick=10;bm.tick=11;am.attackId=bm.attackId=1;
 auto mirror=mr::katanaTrail(am,bm,history);assert(mirror.count==ribbon.count);
 for(std::size_t i=0;i<ribbon.count;++i){assert(std::abs(ribbon.segments[i].to.x+mirror.segments[i].to.x)<.001f);assert(std::abs(ribbon.segments[i].to.y+mirror.segments[i].to.y)<.001f);}
 b.charging=true;assert(mr::katanaTrail(a,b,history).count==0);
 b.charging=false;b.active=false;assert(mr::katanaTrail(a,b,history).count==0);
}

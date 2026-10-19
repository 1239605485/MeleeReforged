#include "render/katana_fan.hpp"
#include "combat/trajectory.hpp"
#include <cassert>
#include <cmath>
int main(){
 auto &w=mr::katana();auto p=mr::evaluatePose(w,0,.34f,{},1,1,34,false);
 auto f=mr::katanaFan(p,.4f);assert(f.visible && f.opacity>0 && f.frame<8);
 auto m=mr::evaluatePose(w,0,.34f,{},-1,1,34,false);auto fm=mr::katanaFan(m,.4f);
 assert(fm.effects==2 && fm.frame==f.frame);assert(std::abs(std::cos(f.rotation)+std::cos(fm.rotation))<.001f);
 p.charging=true;assert(!mr::katanaFan(p,.4f).visible);
 p.charging=false;p.active=false;assert(!mr::katanaFan(p,.4f).visible);
 auto late=mr::evaluatePose(w,0,1,{},1,1,34,false);assert(!mr::katanaFan(late,.4f).visible);
 auto start=mr::evaluatePose(w,0,0,{},1,1,34,false);assert(mr::length(start.tip-late.tip)<.001f);
}

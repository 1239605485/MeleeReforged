#include "combat/sword_wave.hpp"
#include "combat/carry_state.hpp"
#include "weapons/wooden_sword_rules.hpp"
#include "render/wave_renderer.hpp"
#include <cstdio>
#include <cstdlib>
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"line %d: %s\n",__LINE__,#x);std::exit(1);}}while(0)
int main(){
 using namespace mr;
 CHECK(woodenMeleePower(false,0).damage==1);
 CHECK(woodenMeleePower(true,0).damage==2.5f);
 CHECK(woodenMeleePower(true,1).damage==1.3f);
 CHECK(woodenMeleePower(true,2).damage==1.5f);
 CHECK(scaleDamage(10,2.5f)==25 && scaleDamage(0,2.5f)==0);
 CHECK(woodenWavePower.damage==1.2f && woodenWavePower.knockback==2);
 CarryState carry;CHECK(carry.amount()==1);
 carry.advance(true,.1f);CHECK(carry.amount()==0);
 carry.advance(false,.2f);CHECK(carry.amount()==0);
 carry.advance(false,.15f);CHECK(carry.amount()>0 && carry.amount()<1);
 carry.advance(false,.2f);CHECK(carry.amount()==1);
 auto wall=[](Vec2 p){return p.x>=12 && p.x<=28;};
 auto blocked=traceWavePath({0,0},{40,0},wall);
 CHECK(blocked.blocked && blocked.end.x<12);
 CHECK(!traceWavePath({0,0},{8,0},wall).blocked);
 CHECK(traceWavePath({16,0},{40,0},wall).blocked);
 SwordWave wave;CHECK(!wave.active);
 wave.launch({0,0},1,1,12,4);
 CHECK(wave.active && wave.position.x==0);
 auto a=wave.step(1.f/60);CHECK(a.from.x==0 && a.to.x>0 && a.to.x<=4.1f);
 auto fx=WaveRenderer::build(wave);CHECK(fx.count>0 && fx.count<100);
 for(std::size_t i=0;i<fx.count;++i)CHECK(std::isfinite(fx.data[i].to.x) && fx.data[i].color.r==255);
 CHECK(wave.canHit(3,1));wave.markHit(3,1);CHECK(!wave.canHit(3,1));CHECK(wave.canHit(3,2));
 for(int i=0;i<60;++i)wave.step(1.f/60);
 CHECK(!wave.active && wave.distance<=96.01f);
 CHECK(WaveRenderer::build(wave).count==0);
 wave.launch({0,0},-1,-1,12,4);CHECK(wave.step(1.f/60).to.x<0);
 wave.step(-1);CHECK(!wave.active);
 auto right=waistGrip({0,0},20,42,1,1),left=waistGrip({0,0},20,42,-1,1);
 CHECK(right.x+left.x==20 && right.y==left.y);
 std::puts("wooden feature checks passed");
}

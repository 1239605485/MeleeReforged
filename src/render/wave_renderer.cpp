#include "wave_renderer.hpp"
#include <algorithm>
namespace mr {
DrawCommands WaveRenderer::build(const SwordWave &wave){
 DrawCommands out;if(!wave.active)return out;
 float fade=std::clamp(std::min(wave.age*30.f,(96.f-wave.distance)/24.f),0.f,1.f);
 for(int echo=2;echo>=0;--echo)for(int segment=0;segment<12;++segment){
  float a=-1.2f+segment*.2f,b=a+.2f;
  auto point=[&](float angle){return wave.position+Vec2{(std::cos(angle)*20.f-18.f-echo*5.f)*wave.direction,
                                                      std::sin(angle)*20.f*wave.gravity};};
  float alpha=fade*(echo?45.f:210.f);
  out.add({point(a),point(b),echo?4.f:2.f,{255,205,90,static_cast<unsigned char>(alpha)},1});
  if(!echo)out.add({point(a),point(b),.8f,{255,248,210,static_cast<unsigned char>(alpha)},3});
 }
 return out;
}
}

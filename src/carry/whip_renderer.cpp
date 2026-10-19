#include "whip_renderer.hpp"
namespace mr {
DrawCommands coiledWhip(const CarryProfile &profile,Vec2 center,int direction,float gravity,float rotation,float scale){
 DrawCommands out;
 auto point=[&](float x,float y){
  return center+Vec2{(x*std::cos(rotation)-y*std::sin(rotation))*direction*scale,
                     (x*std::sin(rotation)+y*std::cos(rotation))*gravity*scale};
 };
 Color rope{profile.rope[0],profile.rope[1],profile.rope[2],255};
 for(int coil=0;coil<2;++coil)for(int i=0;i<20;++i){
  float a=i*6.2831853f/20,b=(i+1)*6.2831853f/20;
  float rx=5.f-coil*1.8f,ry=7.f-coil*1.8f;
  out.add({point(std::cos(a)*rx,std::sin(a)*ry),point(std::cos(b)*rx,std::sin(b)*ry),1.5f*scale,rope,1});
 }
 out.add({point(3,-5),point(5,-11),2.8f*scale,{110,72,44,255},1});
 out.add({point(4.7f,-10),point(5.3f,-12),3.2f*scale,rope,1});
 return out;
}
}

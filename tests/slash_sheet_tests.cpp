#include "render/slash_sheet.hpp"
#include <cassert>
#include <cmath>
int main() {
 using namespace mr;
 assert(!slashSheet(.1f,false).visible);
 assert(!slashSheet(.8f,false).visible);
 AttackPose attack{};attack.active=true;attack.bladeLength=31;
 SwingProfile fast{.1f,.2f,.7f,-2,1};
 attack.progress=.15f;attack.angle=-1;
 auto timed=slashForAttack(attack,fast);assert(timed.visible);
 auto reverse=fast;reverse.startAngle=1;reverse.endAngle=-2;
 auto rising=slashForAttack(attack,reverse);
 assert(rising.mirrored && !timed.mirrored);
 assert(std::abs(rising.rotation+timed.rotation-2*attack.angle)<.001f);
 attack.progress=.05f;assert(!slashForAttack(attack,fast).visible);
 attack.progress=.7f;assert(!slashForAttack(attack,fast).visible);
 attack.charging=true;attack.progress=.15f;assert(!slashForAttack(attack,fast).visible);
 auto p=slashSheet(.3f,false);assert(p.visible && p.frame>=0 && p.frame<6);
 assert(p.opacity>0 && p.opacity<=1 && p.radius<=58);
 auto a=slashSheet(.3f,false),b=slashSheet(.3f,true);assert(b.radius>a.radius);
 for(int i=0;i<=600;++i) {auto c=slashSheet(i/600.f,false);assert(c.frame>=0 && c.frame<6);}
}

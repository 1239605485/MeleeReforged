#pragma once
#include <algorithm>
#include <cmath>
namespace mr {
struct ButtonIntent { bool holding{}, normal{}, heavy{}; float level{}; };
class AttackButton {
  bool held_{}; float time_{};
public:
  void reset() { held_=false;time_=0; }
  ButtonIntent advance(bool down,float dt) {
    if(!std::isfinite(dt) || dt<0) {reset();return {};}
    if(down) {
      if(!held_) time_=0;
      else time_=std::min(.55f,time_+dt);
      held_=true;return {true,false,false,time_/.55f};
    }
    if(!held_) return {};
    bool full=time_>=.55f-1e-5f;reset();return {false,!full,full,0};
  }
};
}

#pragma once
#include "runtime_api.hpp"
namespace mr {
struct ItemVisual {Handle texture{};NativeRect frame{};explicit operator bool()const{return texture && frame.w>0 && frame.h>0;}};
ItemVisual itemVisual(RuntimeApi &,int itemId);
}

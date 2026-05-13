#pragma once

#include "L2.h"

namespace L2 {
  struct ColorResult {
    bool success; // did the graph coloring succeed
    std::string var; // if failed, what var do we spill
  };

  ColorResult graphColor(Function &f);
}
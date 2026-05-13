#pragma once

#include <unordered_map>
#include <iostream>

#include "L2.h"

namespace L2 {
  // Updates the GEN, KILL, IN, and OUT sets of the function.
  void makeLiveness(Function &f);
}
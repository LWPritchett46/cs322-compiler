#pragma once

#include <vector>

#include "L3.h"

namespace L3 {
  void getContexts(Function *f);
  void treeGen(Context *c);
  void mergeTrees(Function *f);
}
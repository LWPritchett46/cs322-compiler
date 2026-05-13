#pragma once

#include "L2.h"

namespace L2 {
  void parse_file(const char *fileName, Program &p);

  void parse_function(const char *fileName, Function &f);

  std::vector<std::string> parse_for_spill(const char *fileName, Function &f);
}
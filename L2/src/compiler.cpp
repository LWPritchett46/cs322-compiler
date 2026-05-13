#include <algorithm>
#include <assert.h>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <set>
#include <stdint.h>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>

#include <utils.h>

#include "parser.h"
#include "liveness.h"
#include "interference.h"
#include "spill.h"
#include "code_generator.h"

void print_help(char *progName) {
  std::cerr << "Usage: " << progName
            << " [-v] [-g 0|1] [-O 0|1|2] [-s] [-l] [-i] SOURCE" << std::endl;
  return;
}

int main(int argc, char **argv) {
  auto enable_code_generator = true;
  auto spill_only = false;
  auto interference_only = false;
  auto liveness_only = false;
  int32_t optLevel = 3;

  /*
   * Check the compiler arguments.
   */
  Utils::verbose = false;
  if (argc < 2) {
    print_help(argv[0]);
    return 1;
  }
  int32_t opt;
  int64_t functionNumber = -1;
  while ((opt = getopt(argc, argv, "vg:O:slif:")) != -1) {
    switch (opt) {

    case 'l':
      liveness_only = true;
      break;

    case 'i':
      interference_only = true;
      break;

    case 's':
      spill_only = true;
      break;

    case 'O':
      optLevel = strtoul(optarg, NULL, 0);
      break;

    case 'f':
      functionNumber = strtoul(optarg, NULL, 0);
      break;

    case 'g':
      enable_code_generator = (strtoul(optarg, NULL, 0) == 0) ? false : true;
      break;

    case 'v':
      Utils::verbose = true;
      break;

    default:
      print_help(argv[0]);
      return 1;
    }
  }

  L2::Program p;
  L2::Function singleF;

  /*
   * Parse the input file.
   */
  if (spill_only) {

    /*
     * Parse an L2 function and the spill arguments.
     */
    std::vector<std::string> result = L2::parse_for_spill(argv[optind], singleF);
    
    makeSpill(singleF, result[0], result[1], std::cout);

  } else if (liveness_only) {

    /*
     * Parse an L2 function.
     */
    L2::parse_function(argv[optind], singleF);
    makeLiveness(singleF);

  } else if (interference_only) {

    /*
     * Parse an L2 function.
     */
    L2::parse_function(argv[optind], singleF);
    makeLiveness(singleF);
    makeInterference(singleF);

  } else {

    /*
     * Parse the L2 program.
     */
    L2::parse_file(argv[optind], p);
  }

  /*
   * Liveness test.
   */
  if (liveness_only) {

    std::cout << "(\n";
    std::cout << "(in\n";
    for (auto set : singleF.IN) {
      std::cout << "(";
      bool first = true;
      for (auto str : set) {
        if (!first) std::cout << " ";
        std::cout << str;
        first = false;
      }
      std::cout << ")\n";
    }

    std::cout << ")\n\n";
    std::cout << "(out\n";

    for (auto set : singleF.OUT) {
      std::cout << "(";
      bool first = true;
      for (auto str : set) {
        if (!first) std::cout << " ";
        std::cout << str;
        first = false;
      }
      std::cout << ")\n";
    }

    std::cout << ")\n\n)\n";

    return 0;
  }

  /*
   * Interference graph test.
   */
  if (interference_only) {
    for (const auto& [key, values] : singleF.graph) {
      std::cout << key;
      for (const auto& v : values) {
        std::cout << " " << v;
      }
      std::cout << "\n";
    }
    return 0;
  }

  /*
   * Print a single L2 function case.
   */
  if (functionNumber != -1) {
    // TODO

    return 0;
  }

  /*
   * Generate the target code normally.
   */
  if (enable_code_generator) {
    generate_code(p);
  }

  return 0;
}

/* For want of anywhere better, I've decided this is where I will take my notes.
    - current problem tests: 
      - 796 (time out)
      - 811 (array mismatch)
      - 813 (array mismatch)
      - 824 (nothing returned)
      - 859 (array mismatch)
      - 865 (nothing returned)


*/
#include <map>
#include <set>

#include "interference.h"

namespace L2 {

// define a helper function to add edges
void addEdge(const std::string& a, const std::string& b, std::map<std::string, std::set<std::string>>& graph) {
  graph[a].insert(b);
  graph[b].insert(a);
}

void makeInterference(Function &f) {
  // first, add all GPRs to the graph
  std::string genRegs[] = {"rax", "rbx", "rcx", "rdx",
                           "rsi", "rdi", "rbp", // NO RSP
                           "r8",  "r9",  "r10", "r11",
                           "r12", "r13", "r14", "r15"};

  for (const std::string& reg1 : genRegs) {
    for (const std::string& reg2 : genRegs) {
      if (reg1 != reg2) {
        addEdge(reg1, reg2, f.graph);
      }
    }
  }

  // connect based on IN[i]
  for (size_t i = 0; i < f.IN.size(); i++) {
    for (const std::string& var1 : f.IN[i]) {
      for (const std::string& var2 : f.IN[i]) {
        if (var1 != var2) {
          addEdge(var1, var2, f.graph);
        }
      }
    }
  }

  // connect based on OUT[i] and KILL[i]
  for (size_t i = 0; i < f.OUT.size(); i++) {
    for (const std::string& var1 : f.OUT[i]) {
      for (const std::string& var2 : f.OUT[i]) {
        if (var1 != var2) {
          addEdge(var1, var2, f.graph);
        }
      }

      for (const std::string& var2 : f.KILL[i]) {
        if (var1 != var2) {
          addEdge(var1, var2, f.graph);
        }
      }
    }
  }

  // connect based on target constraints (shift operand must be rcx)
  for (size_t i = 0; i < f.instructions.size(); i++) {
    if (auto shft = dynamic_cast<Instruction_shift *>(f.instructions[i])) {
      if (auto var = dynamic_cast<Variable *>(shft->getSrc())) {
        std::string noShiftRegs[] = {"rax", "rbx", "rdx", // NO RCX
                                     "rsi", "rdi", "rbp", // NO RSP
                                     "r8",  "r9",  "r10", "r11",
                                     "r12", "r13", "r14", "r15"};

        for (const std::string& reg : noShiftRegs) {
          addEdge(var->getName(), reg, f.graph);
        }

        // if i didnt have to pass the interference tests this would be unnecessary but oh well
        f.shiftVars.insert(var->getName());
      }
    }
  }
}

}
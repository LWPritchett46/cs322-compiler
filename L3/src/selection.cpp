#include "selection.h"

namespace L3 {

void getContexts(Function *f) {

  Context *current = nullptr;

  auto flushContext = [&]() {
    if (current && !current->instructions.empty()) {
      f->elements.push_back(current);
      current = nullptr;
    }
  };

  for (auto inst : f->instructions) {

    // labels
    if (dynamic_cast<Instruction_label*>(inst)) {
      flushContext();

      auto si = new SoloInstruction();
      si->inst = inst;

      f->elements.push_back(si);

      continue;
    }

    // calls
    if (dynamic_cast<Instruction_call*>(inst) || dynamic_cast<Instruction_call_assign*> (inst)) {
      flushContext();

      auto si = new SoloInstruction();
      si->inst = inst;

      f->elements.push_back(si);

      continue;
    }

    if (!current) {
      current = new Context();
    }

    current->instructions.push_back(inst);

    bool terminates =
            dynamic_cast<Instruction_branch*>(inst) ||
            dynamic_cast<Instruction_branch_cond*>(inst) ||
            dynamic_cast<Instruction_return*>(inst) ||
            dynamic_cast<Instruction_return_val*>(inst);

    if (terminates) {
      flushContext();
    }
  }

  flushContext();

  return;
}

void treeGen(Context *c) {
  for (auto instr : c->instructions) {
    auto tree = instr->generateTree();
    c->forest.push_back(tree);
  }
}

}
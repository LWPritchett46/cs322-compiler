#include "liveness.h"

namespace L2 {
  void makeLiveness(Function &f) {

    // first pass: fill out our nodes
    std::vector<CFGNode *> nodes;
    for (auto instr : f.instructions) {
      auto node = new CFGNode();
      node->instr = instr;
      nodes.push_back(node);

    }


    // second pass: create a label map
    std::unordered_map<std::string, CFGNode *> labelMap;
    for (size_t i = 0; i < nodes.size(); i++) {
      auto lab = dynamic_cast<Instruction_label*>(nodes[i]->instr);
      if (lab) {
        auto item = dynamic_cast<Label*>(lab->getLabel());
        labelMap[item->get()] = nodes[i];
      }
    }


    // third pass: define successors
    for (size_t i = 0; i < nodes.size(); i++) {
      if (dynamic_cast<Instruction_ret*>(nodes[i]->instr)
        || dynamic_cast<Instruction_call_tuple_error*>(nodes[i]->instr)
        || dynamic_cast<Instruction_call_tensor_error*>(nodes[i]->instr)) {

        // No successors, do nothing

      } else if (auto* g = dynamic_cast<Instruction_goto*>(nodes[i]->instr)) {

        // successor of a goto is the label it jumps to
        auto* lbl = dynamic_cast<Label*>(g->getLabel());
        nodes[i]->successors.push_back(labelMap[lbl->get()]);

      } else if (auto* cj = dynamic_cast<Instruction_cjump*>(nodes[i]->instr)) {

        auto* lbl = dynamic_cast<Label*>(cj->getLabel());

        // branch target
        nodes[i]->successors.push_back(labelMap[lbl->get()]);

        // fallthrough
        if (i + 1 < nodes.size()) {
          nodes[i]->successors.push_back(nodes[i + 1]);
        }
      } else {
        if (i + 1 < nodes.size()) {
          nodes[i]->successors.push_back(nodes[i + 1]);
        }
      }
    }


    //create GEN and KILL sets
    for (int i = 0; i < f.instructions.size(); i++) {
      f.GEN.push_back(f.instructions[i]->getGEN());
      f.KILL.push_back(f.instructions[i]->getKILL());
      f.IN.push_back({});
      f.OUT.push_back({});
    }

    std::vector<std::set<std::string>> oldIN = f.IN;
    std::vector<std::set<std::string>> oldOUT = f.OUT;

    std::unordered_map<CFGNode*, size_t> nodeIndex;
    for (size_t i = 0; i < nodes.size(); i++) {
      nodeIndex[nodes[i]] = i;
    }

    do {
      oldIN = f.IN;
      oldOUT = f.OUT;

      for (size_t i = 0; i < nodes.size(); i++) {

        // IN[i] = GEN[i] union (OUT[i] − KILL[i])
        std::set<std::string> newIN = f.GEN[i];

        for (const auto &v : f.OUT[i]) {
          if (f.KILL[i].find(v) == f.KILL[i].end()) {
            newIN.insert(v);
          }
        }

        f.IN[i] = newIN;

        // OUT[i] = union of IN[s] for all successors s
        std::set<std::string> newOUT;
           for (auto succ : nodes[i]->successors) {
          size_t succIndex = nodeIndex[succ];
          newOUT.insert(f.IN[succIndex].begin(), f.IN[succIndex].end());
        }

        f.OUT[i] = newOUT;
      }

    } while (oldIN != f.IN || oldOUT != f.OUT);
  }
}
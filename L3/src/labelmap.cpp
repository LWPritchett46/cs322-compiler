#include <map>
#include <string>

#include "labelmap.h"
#include "L3.h"

namespace L3 {

/*
* Deterministic mangling. Label :labelname in function @functionname becomes 
* :functionname_labelname.
*/

void globalizeLabels(Function *f) {
  std::map<std::string, std::string> labelMap;

  // first pass - map local labels to global labels
  for (auto inst : f->instructions) {
    // all labels are defined in `Instruction_label`s, so we only need to check those.
    if (auto l = dynamic_cast<Instruction_label *>(inst)) {
      auto lbl = dynamic_cast<Label *>(l->getLabel());

      std::string global = ":" + f->name.substr(1) + "_" + lbl->getName();

      labelMap[lbl->getName()] = global;
    }
  }

  // second pass - rewrite all label references
  for (auto inst : f->instructions) {
    inst->rewriteLabels(labelMap);
  }

}

}
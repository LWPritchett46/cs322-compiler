#include <string>
#include <iostream>
#include <fstream>

#include "code_generator.h"
#include "graphcolor.h"
#include "liveness.h"
#include "interference.h"
#include "spill.h"
#include "parser.h"

namespace L2 {

void generate_code(Program &p) {

  std::ofstream outputFile;
  outputFile.open("prog.L1");

  outputFile << "(" << p.entryPointLabel << "\n";
  
  for (auto *f : p.functions) {

    while (true) {

      makeLiveness(*f);
      makeInterference(*f);

      ColorResult result = graphColor(*f);

      if (result.success) {
        break;
      }

      std::ofstream tempFile;
      tempFile.open("temp.L2f");

      std::string prefix = "%SPILL_" + result.var.substr(1) + "_";

      makeSpill(*f, result.var, prefix, tempFile);

      tempFile.close();

      delete f;
      f = new Function();

      parse_function("temp.L2f", *f);
    }

    Context context;
    context.locals = f->locals;
    context.colors = f->colors;
    context.calleeSaves = f->calleeSaves;

    outputFile << "  (" << f->name << "\n";
    outputFile << "    " << f->arguments << " " << (f->locals + f->calleeSaves.size()) << "\n";

    int calleeOffset = (f->calleeSaves.size() + f->locals - 1) * 8;
    for (auto it = f->calleeSaves.begin(); it != f->calleeSaves.end(); it++) {
      outputFile << "    mem rsp " << calleeOffset << " <- " << *it << "\n";
      calleeOffset -= 8;
    }

    for (auto *i : f->instructions) {
      i->generate(outputFile, context);
    }

    outputFile << "  )\n";
  }

  outputFile << ")\n";
  outputFile.close();
}

}
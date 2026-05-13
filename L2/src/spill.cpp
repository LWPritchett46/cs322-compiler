#include <streambuf>

#include "spill.h"

namespace L2 {

class NullBuffer : public std::streambuf
{
  public:
    int overflow(int c) { return c; }
};

void makeSpill(Function &f, std::string var, std::string prefix, std::ostream &out) {

  SpillContext context;
  context.var = var;
  context.prefix = prefix;
  context.count = 0;

  // create a null stream to write the first pass
  NullBuffer null_buffer;
  std::ostream null_stream(&null_buffer);

  // first pass: if there is no spilling, don't increment local count
  for (auto instr : f.instructions) {
    instr->genSpill(null_stream, context);
  }

  if (context.count != 0) {
    context.count = 0;
    f.locals++;
  }

  context.stackAddr = (f.locals - 1) * 8;

  out << "(" << f.name << "\n";
  out << "  " << f.arguments << " " << f.locals << "\n";

  // second pass: generate the spilled function for real
  for (auto instr : f.instructions) {
    instr->genSpill(out, context);
  }

  out << ")\n";
}

}
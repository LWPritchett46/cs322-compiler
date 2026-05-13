#include "L2.h"

namespace L2 {

std::set<std::string> callerSave = {"r10", "r11", "r8", "r9", "rax", "rcx", "rdi", "rdx", "rsi"};
std::set<std::string> calleeSave = {"r12", "r13", "r14", "r15", "rbp", "rbx"};
std::vector<std::string> arguments = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void Register::fillSet(std::set<std::string> &set) const {
  std::string regs[] = {"rax", "rbx", "rcx", "rdx",
                      "rsi", "rdi", "rbp", "rsp",
                      "r8",  "r9",  "r10", "r11",
                      "r12", "r13", "r14", "r15"};

  if (ID != rsp) {
    set.insert(regs[ID]);
  }

  return;
}

std::string Register::format() const {
  std::string regs[] = {"rax", "rbx", "rcx", "rdx",
                      "rsi", "rdi", "rbp", "rsp",
                      "r8",  "r9",  "r10", "r11",
                      "r12", "r13", "r14", "r15"};

  return regs[ID];
}

std::string Register::formatFinal(Context c) const {
  return this->format();
}

Register::Register (RegisterID r)
  : ID {r}{
  return ;
}

void Number::fillSet(std::set<std::string> &set) const {
  // Numbers are constant, no need to fill GEN and KILL
  return;
}

std::string Number::format() const {
  return std::to_string(n);
}

std::string Number::formatFinal(Context c) const {
  return this->format();
}

Number::Number(int64_t n)
  : n {n}{
  return;
}

int64_t Number::get_n() {
  return n;
}

void Label::fillSet(std::set<std::string> &set) const {
  // Ditto with labels
  return;
}

std::string Label::format() const {
  return value;
}

std::string Label::formatFinal(Context c) const {
  return this->format();
}

Label::Label(std::string v) 
  : value {v}{
  return;
}

void Variable::fillSet(std::set<std::string> &set) const {
  // Keep the % in front of variable names
  set.insert(name);
  return;
}

std::string Variable::format() const {
  return name;
}

std::string Variable::formatFinal(Context c) const {
  return c.colors[name];
}

Variable::Variable(std::string name)
  : name {name}{
  return;
}

void Memory::fillSet(std::set<std::string> &set) const {
  base->fillSet(set);
  return;
}

std::string Memory::format() const {
  return "mem " + base->format() + " " + std::to_string(offset);
}

std::string Memory::formatFinal(Context c) const {
  return "mem " + base->formatFinal(c) + " " + std::to_string(offset);
}

std::string Memory::formatSpill(std::string var) const {
  return "mem " + var + " " + std::to_string(offset);
}

Memory::Memory(Item *base, int64_t offset)
  : base {base}, offset {offset} {
  return;
}

Memory::~Memory() {
  delete base;
  return;
}

std::set<std::string> Instruction_ret::getGEN() const {
  std::set<std::string> GEN = {};
  GEN.insert("rax");
  GEN.insert(calleeSave.begin(), calleeSave.end());
  return GEN;
}

std::set<std::string> Instruction_ret::getKILL() const {
  std::set<std::string> KILL = {};
  return KILL;
}

void Instruction_ret::generate(std::ostream &out, Context &c) const {
  int calleeOffset = c.locals * 8;
  for (auto it = c.calleeSaves.rbegin(); it != c.calleeSaves.rend(); it++) {
    out << "    " << *it << " <- mem rsp " << calleeOffset << "\n";
    calleeOffset += 8; 
  }
  out << "    return\n";
}

void Instruction_ret::genSpill(std::ostream &out, SpillContext &c) const {
  out << "  return\n";
  return;
}

std::set<std::string> Instruction_assignment::getGEN() const {
  std::set<std::string> GEN = {};
  src->fillSet(GEN);
  return GEN;
}

std::set<std::string> Instruction_assignment::getKILL() const {
  std::set<std::string> KILL = {};
  dst->fillSet(KILL);
  return KILL;
}

void Instruction_assignment::generate(std::ostream &out, Context &c) const {
  out << "    " << dst->formatFinal(c) << " <- " << src->formatFinal(c) << "\n";
}

void Instruction_assignment::genSpill(std::ostream &out, SpillContext &c) const {
  std::string newVar = c.prefix + std::to_string(c.count);
  if (dst->format() == c.var) {
    if (src->format() == c.var) {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << newVar << " <- " << newVar << "\n";
      out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n";
    } else {
      out << "  " << newVar << " <- " << src->format() << "\n";
      out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n";
    }
    c.count++;
  } else {
    if (src->format() == c.var) {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << dst->format() << " <- " << newVar << "\n";
      c.count++;
    } else {
      out << "  " << dst->format() << " <- " << src->format() << "\n";
    }
  }
}

Instruction_assignment::Instruction_assignment(Item *dst, Item *src)
  : dst {dst}, src {src} {
  return;
}

Instruction_assignment::~Instruction_assignment() {
  delete dst;
  delete src;
}

std::set<std::string> Instruction_store::getGEN() const {
  std::set<std::string> GEN = {};
  mem->fillSet(GEN);
  src->fillSet(GEN);
  return GEN;
}

std::set<std::string> Instruction_store::getKILL() const {
  std::set<std::string> KILL = {};
  return KILL;
}

void Instruction_store::generate(std::ostream &out, Context &c) const {
  out << "    " << mem->formatFinal(c) << " <- " << src->formatFinal(c) << "\n";
}

void Instruction_store::genSpill(std::ostream &out, SpillContext &c) const {
  std::string newVar = c.prefix + std::to_string(c.count);
  if (mem->format().find(c.var) != std::string::npos) {
    if (src->format() == c.var) {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << mem->formatSpill(newVar) << " <- " << newVar << "\n";
    } else {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << mem->formatSpill(newVar) << " <- " << src->format() << "\n";
    }
    c.count++;
  } else {
    if (src->format() == c.var) {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << mem->format() << " <- " << newVar << "\n";
      c.count++;
    } else {
      out << "  " << mem->format() << " <- " << src->format() << "\n";
    }
  }
}

Instruction_store::Instruction_store(Item *mem, Item *src)
  : mem {mem}, src {src} {
  return;
}

Instruction_store::~Instruction_store() {
  delete mem;
  delete src;
  return;
}

std::set<std::string> Instruction_load::getGEN() const {
  std::set<std::string> GEN = {};
  mem->fillSet(GEN);
  return GEN;
}

std::set<std::string> Instruction_load::getKILL() const {
  std::set<std::string> KILL = {};
  dst->fillSet(KILL);
  return KILL;
}

void Instruction_load::generate(std::ostream &out, Context &c) const {
  out << "    " << dst->formatFinal(c) << " <- " << mem->formatFinal(c) << "\n";
}

void Instruction_load::genSpill(std::ostream &out, SpillContext &c) const {
  std::string newVar = c.prefix + std::to_string(c.count);
  if (dst->format() == c.var) {
    if (mem->format().find(c.var) != std::string::npos) {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << newVar << " <- " << mem->formatSpill(newVar) << "\n";
      out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n"; 
    } else {
      out << "  " << newVar << " <- " << mem->format() << "\n";
      out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n";
    }
    c.count++;
  } else {
    if (mem->format().find(c.var) != std::string::npos) {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << dst->format() << " <- " << mem->formatSpill(newVar) << "\n";
      c.count++;
    } else {
      out << "  " << dst->format() << " <- " << mem->format() << "\n";
    }
  }
}

Instruction_load::Instruction_load(Item *dst, Item *mem)
  : dst {dst}, mem {mem} {
  return;
}

Instruction_load::~Instruction_load() {
  delete dst;
  delete mem;
  return;
}

std::set<std::string> Instruction_stack_arg::getGEN() const {
  std::set<std::string> GEN = {};
  return GEN;
}

std::set<std::string> Instruction_stack_arg::getKILL() const {
  std::set<std::string> KILL = {};
  dst->fillSet(KILL);
  return KILL;
}

void Instruction_stack_arg::generate(std::ostream &out, Context &c) const {
  uint64_t newOffset = offset + ((c.locals + c.calleeSaves.size()) * 8);
  out << "    " << dst->formatFinal(c) << " <- mem rsp " << newOffset << "\n";
}

void Instruction_stack_arg::genSpill(std::ostream &out, SpillContext &c) const {
  std::string newVar = c.prefix + std::to_string(c.count);
  if (dst->format() == c.var) {
    out << "  " << newVar << " <- stack-arg " << offset << "\n";
    out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n"; 
    c.count++;
  } else {
    out << "  " << dst->format() << " <- stack-arg " << offset << "\n";
  }
}

Instruction_stack_arg::Instruction_stack_arg(Item *dst, uint64_t offset)
  : dst {dst}, offset {offset} {
  return;
}

Instruction_stack_arg::~Instruction_stack_arg() {
  delete dst;
  return;
}

std::set<std::string> Instruction_arith::getGEN() const {
  std::set<std::string> GEN = {};
  lhs->fillSet(GEN);
  rhs->fillSet(GEN);
  return GEN;
}

std::set<std::string> Instruction_arith::getKILL() const {
  std::set<std::string> KILL = {};
  lhs->fillSet(KILL);
  return KILL;
}

void Instruction_arith::generate(std::ostream &out, Context &c) const {
  std::string opStr[] = {" += ", " -= ", " *= ", " &= "};
  out << "    " << lhs->formatFinal(c) << opStr[op] << rhs->formatFinal(c) << "\n";
}

void Instruction_arith::genSpill(std::ostream &out, SpillContext &c) const {
  std::string newVar = c.prefix + std::to_string(c.count);
  std::string opStr[] = {" += ", " -= ", " *= ", " &= "};
  if (lhs->format() == c.var) {
    if (rhs->format() == c.var) {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << newVar << opStr[op] << newVar << "\n";
      out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n"; 
    } else {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << newVar << opStr[op] << rhs->format() << "\n";
      out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n";
    }
    c.count++;
  } else {
    if (rhs->format() == c.var) {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << lhs->format() << opStr[op] << newVar << "\n";
      c.count++;
    } else {
      out << "  " << lhs->format() << opStr[op] << rhs->format() << "\n";
    }
  }
}

Instruction_arith::Instruction_arith(Item *lhs, ArithOp op, Item *rhs)
  : lhs {lhs}, op {op}, rhs {rhs} {
  return;
}

Instruction_arith::~Instruction_arith() {
  delete lhs;
  delete rhs;
  return;
}

std::set<std::string> Instruction_inc::getGEN() const {
  std::set<std::string> GEN = {};
  val->fillSet(GEN);
  return GEN;
}

std::set<std::string> Instruction_inc::getKILL() const {
  std::set<std::string> KILL = {};
  val->fillSet(KILL);
  return KILL;
}

void Instruction_inc::generate(std::ostream &out, Context &c) const {
  out << "    " << val->formatFinal(c) << "++\n";
}

void Instruction_inc::genSpill(std::ostream &out, SpillContext &c) const {
  std::string newVar = c.prefix + std::to_string(c.count);
  if (val->format() == c.var) {
    out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
    out << "  " << newVar << "++\n";
    out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n";
    c.count++;
  } else {
    out << "  " << val->format() << "++\n";
  }
}

Instruction_inc::Instruction_inc(Item *val)
  : val {val} {
  return;
}

Instruction_inc::~Instruction_inc() {
  delete val;
  return;
}

std::set<std::string> Instruction_dec::getGEN() const {
  std::set<std::string> GEN = {};
  val->fillSet(GEN);
  return GEN;
}

std::set<std::string> Instruction_dec::getKILL() const {
  std::set<std::string> KILL = {};
  val->fillSet(KILL);
  return KILL;
}

void Instruction_dec::generate(std::ostream &out, Context &c) const {
  out << "    " << val->formatFinal(c) << "--\n";
}

void Instruction_dec::genSpill(std::ostream &out, SpillContext &c) const {
  std::string newVar = c.prefix + std::to_string(c.count);
  if (val->format() == c.var) {
    out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
    out << "  " << newVar << "--\n";
    out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n";
    c.count++;
  } else {
    out << "  " << val->format() << "--\n";
  }
}

Instruction_dec::Instruction_dec(Item *val)
  : val {val} {
  return;
}

Instruction_dec::~Instruction_dec() {
  delete val;
  return;
}

std::set<std::string> Instruction_mem_rhs::getGEN() const {
  std::set<std::string> GEN = {};
  mem->fillSet(GEN);
  lhs->fillSet(GEN);
  return GEN;
}

std::set<std::string> Instruction_mem_rhs::getKILL() const {
  std::set<std::string> KILL = {};
  lhs->fillSet(KILL);
  return KILL;
}

void Instruction_mem_rhs::generate(std::ostream &out, Context &c) const {
  std::string opStr[] = {" += ", " -= ", " *= ", " &= "};
  out << "    " << lhs->formatFinal(c) << opStr[op] << mem->formatFinal(c) << "\n";
}

void Instruction_mem_rhs::genSpill(std::ostream &out, SpillContext &c) const {
  std::string newVar = c.prefix + std::to_string(c.count);
  std::string opStr[] = {" += ", " -= "};
  if (lhs->format() == c.var) {
    if (mem->format().find(c.var) != std::string::npos) {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << newVar << opStr[op] << mem->formatSpill(newVar) << "\n";
      out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n";
    } else {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << newVar << opStr[op] << mem->format() << "\n";
      out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n";
    }
    c.count++;
  } else {
    if (mem->format().find(c.var) != std::string::npos) {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << lhs->format() << opStr[op] << mem->formatSpill(newVar) << "\n";
      c.count++;
    } else {
      out << "  " << lhs->format() << opStr[op] << mem->format() << "\n";
    }
  }
}

Instruction_mem_rhs::Instruction_mem_rhs(Item *lhs, ArithOp op, Item *mem)
  : lhs {lhs}, op {op}, mem {mem} {
  return;
}

Instruction_mem_rhs::~Instruction_mem_rhs() {
  delete lhs;
  delete mem;
  return;
}

std::set<std::string> Instruction_mem_lhs::getGEN() const {
  std::set<std::string> GEN = {};
  mem->fillSet(GEN);
  rhs->fillSet(GEN);
  return GEN;
}

std::set<std::string> Instruction_mem_lhs::getKILL() const {
  std::set<std::string> KILL = {};
  return KILL;
}

void Instruction_mem_lhs::generate(std::ostream &out, Context &c) const {
  std::string opStr[] = {" += ", " -= ", " *= ", " &= "};
  out << "    " << mem->formatFinal(c) << opStr[op] << rhs->formatFinal(c) << "\n";
}

void Instruction_mem_lhs::genSpill(std::ostream &out, SpillContext &c) const {
  std::string newVar = c.prefix + std::to_string(c.count);
  std::string opStr[] = {" += ", " -= "};
  if (mem->format().find(c.var) != std::string::npos) {
    if (rhs->format() == c.var) {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << mem->formatSpill(newVar) << opStr[op] << newVar << "\n";
    } else {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << mem->formatSpill(newVar) << opStr[op] << rhs->format() << "\n";
    }
    c.count++;
  } else {
    if (rhs->format() == c.var) {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << mem->format() << opStr[op] << newVar << "\n";
      c.count++;
    } else {
      out << "  " << mem->format() << opStr[op] << rhs->format() << "\n";
    }
  }
}

Instruction_mem_lhs::Instruction_mem_lhs(Item *mem, ArithOp op, Item *rhs)
  : mem {mem}, op {op}, rhs {rhs} {
  return;
}

Instruction_mem_lhs::~Instruction_mem_lhs() {
  delete mem;
  delete rhs;
  return;
}

std::set<std::string> Instruction_compare_assign::getGEN() const {
  std::set<std::string> GEN = {};
  lhs->fillSet(GEN);
  rhs->fillSet(GEN);
  return GEN;
}

std::set<std::string> Instruction_compare_assign::getKILL() const {
  std::set<std::string> KILL = {};
  dst->fillSet(KILL);
  return KILL;
}

void Instruction_compare_assign::generate(std::ostream &out, Context &c) const {
  std::string opStr[] = {" < ", " <= ", " = "};
  out << "    " << dst->formatFinal(c) << " <- " << lhs->formatFinal(c) << opStr[op] << rhs->formatFinal(c) << "\n";
}

void Instruction_compare_assign::genSpill(std::ostream &out, SpillContext &c) const {
  std::string newVar = c.prefix + std::to_string(c.count);
  std::string opStr[] = {" < ", " <= ", " = "};
  if (dst->format() == c.var) {
    if (lhs->format() == c.var) {
      if (rhs->format() == c.var) {
        out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
        out << "  " << newVar << " <- " << newVar << opStr[op] << newVar << "\n";
        out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n";
      } else {
        out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
        out << "  " << newVar << " <- " << newVar << opStr[op] << rhs->format() << "\n";
        out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n";
      }
    } else {
      if (rhs->format() == c.var) {
        out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
        out << "  " << newVar << " <- " << lhs->format() << opStr[op] << newVar << "\n";
        out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n";
      } else {
        out << "  " << newVar << " <- " << lhs->format() << opStr[op] << rhs->format() << "\n";
        out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n";
      }
    }
    c.count++;
  } else {
    if (lhs->format() == c.var) {
      if (rhs->format() == c.var) {
        out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
        out << "  " << dst->format() << " <- " << newVar << opStr[op] << newVar << "\n";
      } else {
        out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
        out << "  " << dst->format() << " <- " << newVar << opStr[op] << rhs->format() << "\n";
      }
      c.count++;
    } else {
      if (rhs->format() == c.var) {
        out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
        out << "  " << dst->format() << " <- " << lhs->format() << opStr[op] << newVar << "\n";
        c.count++;
      } else {
        out << "  " << dst->format() << " <- " << lhs->format() << opStr[op] << rhs->format() << "\n";
      }
    }
  }
}

Instruction_compare_assign::Instruction_compare_assign(Item *dst, Item *lhs, CompareOp op, Item *rhs)
  : dst {dst}, lhs {lhs}, op {op}, rhs {rhs} {
  return;
}

Instruction_compare_assign::~Instruction_compare_assign() {
  delete dst;
  delete lhs;
  delete rhs;
  return;
}

std::set<std::string> Instruction_label::getGEN() const {
  std::set<std::string> GEN = {};
  return GEN;
}

std::set<std::string> Instruction_label::getKILL() const {
  std::set<std::string> KILL = {};
  return KILL;
}

void Instruction_label::generate(std::ostream &out, Context &c) const {
  out << "  " << lbl->formatFinal(c) << "\n";
}

void Instruction_label::genSpill(std::ostream &out, SpillContext &c) const {
  out << "  " << lbl->format() << "\n";
}

Instruction_label::Instruction_label(Item *lbl) 
  : lbl {lbl} {
  return;
}

Instruction_label::~Instruction_label() {
  delete lbl;
  return;
}

std::set<std::string> Instruction_goto::getGEN() const {
  std::set<std::string> GEN = {};
  return GEN;
}

std::set<std::string> Instruction_goto::getKILL() const {
  std::set<std::string> KILL = {};
  return KILL;
}

void Instruction_goto::generate(std::ostream &out, Context &c) const {
  out << "    goto " << lbl->formatFinal(c) << "\n";
}

void Instruction_goto::genSpill(std::ostream &out, SpillContext &c) const {
  out << "  goto " << lbl->format() << "\n";
}

Instruction_goto::Instruction_goto(Item *lbl) 
  : lbl {lbl} {
  return;
}

Instruction_goto::~Instruction_goto() {
  delete lbl;
  return;
}

std::set<std::string> Instruction_cjump::getGEN() const {
  std::set<std::string> GEN = {};
  lhs->fillSet(GEN);
  rhs->fillSet(GEN);
  return GEN;
}

std::set<std::string> Instruction_cjump::getKILL() const {
  std::set<std::string> KILL = {};
  return KILL;
}

void Instruction_cjump::generate(std::ostream &out, Context &c) const {
  std::string opStr[] = {" < ", " <= ", " = "};
  out << "    cjump " << lhs->formatFinal(c) << opStr[op] << rhs->formatFinal(c) << " " << lbl->formatFinal(c) << "\n";
}

void Instruction_cjump::genSpill(std::ostream &out, SpillContext &c) const {
  std::string newVar = c.prefix + std::to_string(c.count);
  std::string opStr[] = {" < ", " <= ", " = "};
  if (lhs->format() == c.var) {
    if (rhs->format() == c.var) {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  cjump " << newVar << opStr[op] << newVar << " " << lbl->format() << "\n";
    } else {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  cjump " << newVar << opStr[op] << rhs->format() << " " << lbl->format() << "\n";
    }
    c.count++;
  } else {
    if (rhs->format() == c.var) {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  cjump " << lhs->format() << opStr[op] << newVar << " " << lbl->format() << "\n";
      c.count++;
    } else {
      out << "  cjump " << lhs->format() << opStr[op] << rhs->format() << " " << lbl->format() << "\n";
    }
  }
}

Instruction_cjump::Instruction_cjump(Item *lhs, CompareOp op, Item *rhs, Item *lbl)
  : lhs {lhs}, op {op}, rhs {rhs}, lbl {lbl} {
  return;
}

Instruction_cjump::~Instruction_cjump() {
  delete lhs;
  delete rhs;
  delete lbl;
  return;
}

std::set<std::string> Instruction_shift::getGEN() const {
  std::set<std::string> GEN = {};
  val->fillSet(GEN);
  src->fillSet(GEN);
  return GEN;
}

std::set<std::string> Instruction_shift::getKILL() const {
  std::set<std::string> KILL = {};
  val->fillSet(KILL);
  return KILL;
}

void Instruction_shift::generate(std::ostream &out, Context &c) const {
  std::string opStr[] = {" <<= ", " >>= "};
  out << "    " << val->formatFinal(c) << opStr[op] << src->formatFinal(c) << "\n";
}

void Instruction_shift::genSpill(std::ostream &out, SpillContext &c) const {
  std::string newVar = c.prefix + std::to_string(c.count);
  std::string opStr[] = {" <<= ", " >>= "};
  if (val->format() == c.var) {
    if (src->format() == c.var) {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << newVar << opStr[op] << newVar << "\n";
      out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n"; 
    } else {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << newVar << opStr[op] << src->format() << "\n";
      out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n";
    }
    c.count++;
  } else {
    if (src->format() == c.var) {
      out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
      out << "  " << val->format() << opStr[op] << newVar << "\n";
      c.count++;
    } else {
      out << "  " << val->format() << opStr[op] << src->format() << "\n";
    }
  }
}

Instruction_shift::Instruction_shift(Item *val, ShiftOp op, Item *src)
  : val {val}, op {op}, src {src} {
  return;
}

Instruction_shift::~Instruction_shift() {
  delete val;
  delete src;
  return;
}

std::set<std::string> Instruction_lea::getGEN() const {
  std::set<std::string> GEN = {};
  src1->fillSet(GEN);
  src2->fillSet(GEN);
  return GEN;
}

std::set<std::string> Instruction_lea::getKILL() const {
  std::set<std::string> KILL = {};
  dst->fillSet(KILL);
  return KILL;
}

void Instruction_lea::generate(std::ostream &out, Context &c) const {
  out << "    " << dst->formatFinal(c) << " @ " << src1->formatFinal(c) << " " 
      << src2->formatFinal(c) << " " << offset << "\n";
}

void Instruction_lea::genSpill(std::ostream &out, SpillContext &c) const {
  std::string newVar = c.prefix + std::to_string(c.count);
  if (dst->format() == c.var) {
    if (src1->format() == c.var) {
      if (src2->format() == c.var) {
        out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
        out << "  " << newVar << " @ " << newVar << " " << newVar << " " << offset << "\n";
        out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n";
      } else {
        out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
        out << "  " << newVar << " @ " << newVar << " " << src2->format() << " " << offset << "\n";
        out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n";
      }
    } else {
      if (src2->format() == c.var) {
        out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
        out << "  " << newVar << " @ " << src1->format() << " " << newVar << " " << offset << "\n";
        out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n";
      } else {
        out << "  " << newVar << " @ " << src1->format() << " " << src2->format() << " " << offset << "\n";
        out << "  mem rsp " << c.stackAddr << " <- " << newVar << "\n";
      }
    }
    c.count++;
  } else {
    if (src1->format() == c.var) {
      if (src2->format() == c.var) {
        out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
        out << "  " << dst->format() << " @ " << newVar << " " << newVar << " " << offset << "\n";
      } else {
        out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
        out << "  " << dst->format() << " @ " << newVar << " " << src2->format() << " " << offset << "\n";
      }
      c.count++;
    } else {
      if (src2->format() == c.var) {
        out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
        out << "  " << dst->format() << " @ " << src1->format() << " " << newVar << " " << offset << "\n";
        c.count++;
      } else {
        out << "  " << dst->format() << " @ " << src1->format() << " " << src2->format() << " " << offset << "\n";
      }
    }
  }
}

Instruction_lea::Instruction_lea(Item *dst, Item *src1, Item *src2, uint64_t offset)
  : dst {dst}, src1 {src1}, src2 {src2}, offset {offset} {
  return;
}

Instruction_lea::~Instruction_lea() {
  delete dst;
  delete src1;
  delete src2;
  return;
}

std::set<std::string> Instruction_call::getGEN() const {
  std::set<std::string> GEN = {};
  func->fillSet(GEN);
  for (int i = 0; i < std::min(args, 6ul); i++) {
    GEN.insert(arguments[i]);
  }
  return GEN;
}

std::set<std::string> Instruction_call::getKILL() const {
  std::set<std::string> KILL = {};
  KILL.insert(callerSave.begin(), callerSave.end());
  return KILL;
}

void Instruction_call::generate(std::ostream &out, Context &c) const {
  out << "    call " << func->formatFinal(c) << " " << args << "\n";
}

void Instruction_call::genSpill(std::ostream &out, SpillContext &c) const {
  std::string newVar = c.prefix + std::to_string(c.count);
  if (func->format() == c.var) {
    out << "  " << newVar << " <- mem rsp " << c.stackAddr << "\n";
    out << "  call " << newVar << " " << args << "\n";
    c.count++;
  } else {
    out << "  call " << func->format() << " " << args << "\n";
  }
}

Instruction_call::Instruction_call(Item *func, uint64_t args)
  : func {func}, args {args} {
  return;
}

Instruction_call::~Instruction_call() {
  delete func;
  return;
}

std::set<std::string> Instruction_call_print::getGEN() const {
  std::set<std::string> GEN = {};
  GEN.insert("rdi");
  return GEN;
}

std::set<std::string> Instruction_call_print::getKILL() const {
  std::set<std::string> KILL = {};
  KILL.insert(callerSave.begin(), callerSave.end());
  return KILL;
}

void Instruction_call_print::generate(std::ostream &out, Context &c) const {
  out << "    call print 1\n";
}

void Instruction_call_print::genSpill(std::ostream &out, SpillContext &c) const {
  out << "  call print 1\n";
}

std::set<std::string> Instruction_call_input::getGEN() const {
  std::set<std::string> GEN = {};
  return GEN;
}

std::set<std::string> Instruction_call_input::getKILL() const {
  std::set<std::string> KILL = {};
  KILL.insert(callerSave.begin(), callerSave.end());
  return KILL;
}

void Instruction_call_input::generate(std::ostream &out, Context &c) const {
  out << "    call input 0\n";
}

void Instruction_call_input::genSpill(std::ostream &out, SpillContext &c) const {
  out << "  call input 0\n";
}

std::set<std::string> Instruction_call_allocate::getGEN() const {
  std::set<std::string> GEN = {};
  GEN.insert({"rdi", "rsi"});
  return GEN;
}

std::set<std::string> Instruction_call_allocate::getKILL() const {
  std::set<std::string> KILL = {};
  KILL.insert(callerSave.begin(), callerSave.end());
  return KILL;
}

void Instruction_call_allocate::generate(std::ostream &out, Context &c) const {
  out << "    call allocate 2\n";
}

void Instruction_call_allocate::genSpill(std::ostream &out, SpillContext &c) const {
  out << "  call allocate 2\n";
}

std::set<std::string> Instruction_call_tuple_error::getGEN() const {
  std::set<std::string> GEN = {};
  GEN.insert({"rdi", "rsi", "rdx"});
  return GEN;
}

std::set<std::string> Instruction_call_tuple_error::getKILL() const {
  std::set<std::string> KILL = {};
  KILL.insert(callerSave.begin(), callerSave.end());
  return KILL;
}

void Instruction_call_tuple_error::generate(std::ostream &out, Context &c) const {
  out << "    call tuple-error 3\n";
}

void Instruction_call_tuple_error::genSpill(std::ostream &out, SpillContext &c) const {
  out << "  call tuple-error 3\n";
}

std::set<std::string> Instruction_call_tensor_error::getGEN() const {
  std::set<std::string> GEN = {};
  for (int i = 0; i < args; i++) {
    GEN.insert(arguments[i]);
  }
  return GEN;
}

std::set<std::string> Instruction_call_tensor_error::getKILL() const {
  std::set<std::string> KILL = {};
  KILL.insert(callerSave.begin(), callerSave.end());
  return KILL;
}

void Instruction_call_tensor_error::generate(std::ostream &out, Context &c) const {
  out << "    call tensor-error " << args << "\n";
}

void Instruction_call_tensor_error::genSpill(std::ostream &out, SpillContext &c) const {
  out << "  call tensor-error " << args << "\n";
}

Instruction_call_tensor_error::Instruction_call_tensor_error(uint64_t args)
  : args {args} {
  return;
}

Function::~Function() {
  for (auto i : instructions) {
    delete i;
  }
  return;
}

Program::~Program() {
  for (auto f : functions) {
    delete f;
  }
  return;
}

}
#include <L1.h>
#include <iostream>
#include <algorithm>

namespace L1 {

std::string Register::format() const {
  static const std::string names[] = {
    "%rax", "%rbx", "%rcx", "%rdx",
    "%rsi", "%rdi", "%rbp", "%rsp",
    "%r8",  "%r9",  "%r10", "%r11",
    "%r12", "%r13", "%r14", "%r15"
  };

  return names[ID];
}

std::string Register::format8() const {
  static const std::string names[] = {
    "%al",   "%bl",   "%cl",   "%dl",
    "%sil",  "%dil",  "%bpl",  "%spl",
    "%r8b",  "%r9b",  "%r10b", "%r11b",
    "%r12b", "%r13b", "%r14b", "%r15b"
  };

  return names[ID];
}

Register::Register (RegisterID r)
  : ID {r}{
  return ;
}

std::string Number::format() const {
  return "$" + std::to_string(n);
}

Number::Number(int64_t n)
  : n {n}{
  return;
}

int64_t Number::get_n() {
  return n;
}

std::string Label::format() const {
  return "_" + value.substr(1);
}

Label::Label(std::string v) 
  : value {v}{
  return;
}

std::string Memory::format() const {
  return std::to_string(offset) + "(" + base->format() + ")";
}

Memory::Memory(Item *base, int64_t offset)
  : base {base}, offset {offset} {
  return;
}

Memory::~Memory() {
  delete base;
  return;
}

void Instruction_ret::generate(std::ostream& out, Context c) const {
  uint64_t extra_args;
  if (c.args > 6) { 
    extra_args = c.args - 6; 
  }
  else { 
    extra_args = 0; 
  }
  uint64_t offset = (extra_args + c.locals) * 8;

  if (offset != 0) {
    out << "  addq $" << offset << ", %rsp\n";
  }
  out << "  retq\n";
}

void Instruction_assignment::generate(std::ostream& out, Context c) const {
  std::string srcStr = src->format();
  if (srcStr.at(0) == '_') {
    srcStr = "$" + srcStr;
  }
  std::string dstStr = dst->format();

  out << "  movq " << srcStr << ", " << dstStr << "\n";
}

Instruction_assignment::Instruction_assignment (Item *dst, Item *src)
  : src { src },
    dst { dst } {
  return ;
}

Instruction_assignment::~Instruction_assignment() {
  delete src;
  delete dst;
  return;
}

void Instruction_store::generate(std::ostream& out, Context c) const {
  std::string srcStr = src->format();
  if (srcStr.at(0) == '_') {
    srcStr = "$" + srcStr;
  }
  std::string memStr = mem->format();

  out << "  movq " << srcStr << ", " << memStr << "\n";
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

void Instruction_load::generate(std::ostream& out, Context c) const {
  std::string memStr = mem->format();
  std::string dstStr = dst->format();

  out << "  movq " << memStr << ", " << dstStr << "\n";
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

void Instruction_arith::generate(std::ostream& out, Context c) const {
  static const std::string instructions[] = {
    "addq", "subq", "imulq", "andq"
  };

  std::string lhsStr = lhs->format();
  std::string rhsStr = rhs->format();

  out << "  " << instructions[op] << " " << rhsStr << ", " << lhsStr << "\n";
  
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

void Instruction_inc::generate(std::ostream& out, Context c) const {
  std::string valStr = val->format();

  out << "  inc " << valStr << "\n";
}

Instruction_inc::Instruction_inc(Item *val)
  : val {val} {
  return;
}

Instruction_inc::~Instruction_inc() {
  delete val;
  return;
}

void Instruction_dec::generate(std::ostream& out, Context c) const {
  std::string valStr = val->format();

  out << "  dec " << valStr << "\n";
}

Instruction_dec::Instruction_dec(Item *val)
  : val {val} {
  return;
}

Instruction_dec::~Instruction_dec() {
  delete val;
  return;
}

void Instruction_mem_rhs::generate(std::ostream& out, Context c) const {
  static const std::string instructions[] = {
    "addq", "subq", "imulq", "andq"
  };

  std::string lhsStr = lhs->format();
  std::string memStr = mem->format();

  out << "  " << instructions[op] << " " << memStr << ", " << lhsStr << "\n";
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

void Instruction_mem_lhs::generate(std::ostream& out, Context c) const {
  static const std::string instructions[] = {
    "addq", "subq", "imulq", "andq"
  };

  std::string memStr = mem->format();
  std::string rhsStr = rhs->format();

  out << "  " << instructions[op] << " " << rhsStr << ", " << memStr << "\n";
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

void Instruction_compare_assign::generate(std::ostream& out, Context c) const {
  std::string dstStr = dst->format();
  std::string dst8   = dst->format8();
  std::string lhsStr = lhs->format();
  std::string rhsStr = rhs->format();

  if (lhsStr[0] == '$' && rhsStr[0] == '$') {
    uint64_t lhsVal = std::stoll(lhsStr.substr(1));
    uint64_t rhsVal = std::stoll(rhsStr.substr(1));
    switch (op) {
      case EQ:
        if (lhsVal == rhsVal) {
          out << "  movq $1, " << dstStr << "\n";
        } else {
          out << "  movq $0, " << dstStr << "\n";
        }
        break;

      case LEQ:
        if (lhsVal <= rhsVal) {
          out << "  movq $1, " << dstStr << "\n";
        } else {
          out << "  movq $0, " << dstStr << "\n";
        }
        break;

      case LT:
        if (lhsVal < rhsVal) {
          out << "  movq $1, " << dstStr << "\n";
        } else {
          out << "  movq $0, " << dstStr << "\n";
        }
        break;
    }
  } else if (lhsStr[0] == '$') {
    out << "  cmpq " << lhsStr << ", " << rhsStr << "\n";

    switch (op) {
      case EQ:
        out << "  sete " << dst8 << "\n";
        break;
      case LEQ: 
        out << "  setge " << dst8 << "\n";
        break;
      case LT:
        out << "  setg " << dst8 << "\n";
        break;
    }

    out << "  movzbq " << dst8 << ", " << dstStr << "\n";

  } else {
    out << "  cmpq " << rhsStr << ", " << lhsStr << "\n";

    switch (op) {
      case EQ:
        out << "  sete " << dst8 << "\n";
        break;
      case LEQ: 
        out << "  setle " << dst8 << "\n";
        break;
      case LT:
        out << "  setl " << dst8 << "\n";
        break;
    }

    out << "  movzbq " << dst8 << ", " << dstStr << "\n";
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

void Instruction_label::generate(std::ostream& out, Context c) const {
  std::string lblStr = lbl->format();

  out << lblStr << ":\n";
}

Instruction_label::Instruction_label(Item *lbl) 
  : lbl {lbl} {
  return;
}

Instruction_label::~Instruction_label() {
  delete lbl;
  return;
}

void Instruction_goto::generate(std::ostream& out, Context c) const {
  std::string lblStr = lbl->format();

  out << "  jmp " << lblStr << "\n";
}

Instruction_goto::Instruction_goto(Item *lbl) 
  : lbl {lbl} {
  return;
}

Instruction_goto::~Instruction_goto() {
  delete lbl;
  return;
}

void Instruction_cjump::generate(std::ostream& out, Context c) const {
  std::string lhsStr = lhs->format();
  std::string rhsStr = rhs->format();
  std::string lblStr = lbl->format();

  if (lhsStr[0] == '$' && rhsStr[0] == '$') {
    uint64_t lhsVal = std::stoll(lhsStr.substr(1));
    uint64_t rhsVal = std::stoll(rhsStr.substr(1));
    switch (op) {
      case EQ:
        if (lhsVal == rhsVal) {
          out << "  jmp " << lblStr << "\n";
        }
        break;
      case LEQ:
        if (lhsVal <= rhsVal) {
          out << "  jmp " << lblStr << "\n";
        }
        break;
      case LT:
        if (lhsVal < rhsVal) {
          out << "  jmp " << lblStr << "\n";
        }
    }
  } else if (lhsStr[0] == '$') {
    out << "  cmpq " << lhsStr << ", " << rhsStr << "\n";
    switch (op) {
      case EQ:
        out << "  je " << lblStr << "\n";
        break;
      case LEQ:
        out << "  jge " << lblStr << "\n";
        break;
      case LT:
        out << "  jg " << lblStr << "\n";
        break;
    }
  } else {
    out << "  cmpq " << rhsStr << ", " << lhsStr << "\n";
    switch (op) {
      case EQ:
        out << "  je " << lblStr << "\n";
        break;
      case LEQ:
        out << "  jle " << lblStr << "\n";
        break;
      case LT:
        out << "  jl " << lblStr << "\n";
        break;
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

void Instruction_shift::generate(std::ostream &out, Context c) const {
  std::string valStr = val->format();
  std::string srcStr = src->format();
  if (srcStr[0] == '%') {
    srcStr = src->format8();
  }

  if (op == SHL) {
    out << "  salq ";
  } else {
    out << "  sarq ";
  }

  out << srcStr << ", " << valStr << "\n";
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

void Instruction_lea::generate(std::ostream& out, Context c) const {
  std::string src1Str = src1->format();
  std::string src2Str = src2->format();
  std::string dstStr = dst->format();
  out << "  lea (" << src1Str << ", " << src2Str << ", " << offset << "), " << dstStr << "\n";
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

void Instruction_call::generate(std::ostream& out, Context c) const {
  std::string funcStr = func->format();
  if (funcStr[0] == '%') {
    funcStr = "*" + funcStr;
  }
  
  uint64_t extraArgs = 0;
  if (args > 6) {
    extraArgs = args - 6;
  } 
  uint64_t offset = (extraArgs + 1) * 8;

  out << "  subq $" << offset << ", %rsp\n";
  out << "  jmp " << funcStr << "\n";
}

Instruction_call::Instruction_call(Item *func, uint64_t args)
  : func {func}, args {args} {
  return;
}

Instruction_call::~Instruction_call() {
  delete func;
  return;
}

void Instruction_call_print::generate(std::ostream& out, Context c) const {
  out << "  call print\n";
}

void Instruction_call_allocate::generate(std::ostream& out, Context c) const {
  out << "  call allocate\n";
}

void Instruction_call_input::generate(std::ostream& out, Context c) const {
  out << "  call input\n";
}

void Instruction_call_tuple_error::generate(std::ostream& out, Context c) const {
  out << "  call tuple_error\n";
}

void Instruction_call_tensor_error::generate(std::ostream& out, Context c) const {
  switch (args) {
    case 1:
      out << "  call array_tensor_error_null\n";
      return;
    case 3:
      out << "  call array_error\n";
      return;
    case 4:
      out << "  call tensor_error\n";
  }
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

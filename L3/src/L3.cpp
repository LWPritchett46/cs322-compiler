#include "L3.h"

namespace L3 {

Number::Number(int64_t value) 
  : value {value} {
  return;
}

Variable::Variable(std::string name)
  : name {name} {
  return;
}

Label::Label(std::string lbl)
  : lbl {lbl} {
  return;
}

ItemList::ItemList(std::vector<Item *> items)
  : items {items} {
  return;
}

ItemList::~ItemList() {
  for (auto item : items) {
    delete item;
  }
  return;
}

Instruction_return_val::Instruction_return_val(Item *val)
  : val {val} {
  return;
}

Instruction_return_val::~Instruction_return_val() {
  delete val;
  return;
}

Instruction_assignment::Instruction_assignment(Item *dst, Item *src)
  : dst {dst}, src {src} {
  return;
}

Instruction_assignment::~Instruction_assignment() {
  delete dst;
  delete src;
  return;
}

Instruction_op_assign::Instruction_op_assign(Item *dst, Item *lhs, Op op, Item *rhs)
  : dst {dst}, lhs {lhs}, op {op}, rhs {rhs} {
  return;
}

Instruction_op_assign::~Instruction_op_assign() {
  delete dst;
  delete lhs;
  delete rhs;
  return;
}

Instruction_cmp_assign::Instruction_cmp_assign(Item *dst, Item *lhs, Cmp cmp, Item *rhs)
  : dst {dst}, lhs {lhs}, cmp {cmp}, rhs {rhs} {
  return;
}

Instruction_cmp_assign::~Instruction_cmp_assign() {
  delete dst;
  delete lhs;
  delete rhs;
  return;
}

Function::~Function() {
  for (auto a : arguments) {
    delete a;
  }
  for (auto i : instructions) {
    delete i;
  }
  return;
}

Program::~Program() {
  for (auto f : functions) {
    delete f;
  }
}

}
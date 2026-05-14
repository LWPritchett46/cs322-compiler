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

Instruction_load::Instruction_load(Item *dst, Item *ptr)
  : dst {dst}, ptr {ptr} {
  return;
}

Instruction_load::~Instruction_load() {
  delete dst;
  delete ptr;
  return;
}

Instruction_store::Instruction_store(Item *ptr, Item *src)
  : ptr {ptr}, src {src} {
  return;
}

Instruction_store::~Instruction_store() {
  delete ptr;
  delete src;
  return;
}

Instruction_label::Instruction_label(Item *lbl)
  : lbl {lbl} {
  return;
}

Instruction_label::~Instruction_label() {
  delete lbl;
  return;
}

Instruction_branch::Instruction_branch(Item *lbl) 
  : lbl {lbl} {
  return;
}

Instruction_branch::~Instruction_branch() {
  delete lbl;
  return;
}

Instruction_branch_cond::Instruction_branch_cond(Item *cond, Item *lbl)
  : cond {cond}, lbl {lbl} {
  return;
}

Instruction_branch_cond::~Instruction_branch_cond() {
  delete cond;
  delete lbl;
  return;
}

Instruction_call::Instruction_call(Item *callee, ItemList *args)
  : callee {callee}, args {args} {
  return;
}

Instruction_call::~Instruction_call() {
  delete callee;
  delete args;
  return;
}

Instruction_call_assign::Instruction_call_assign(Item *dst, Item *callee, ItemList *args)
  : dst {dst}, callee {callee}, args {args} {
  return;
}

Instruction_call_assign::~Instruction_call_assign() {
  delete dst;
  delete callee;
  delete args;
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
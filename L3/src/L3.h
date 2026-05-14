#pragma once

#include <vector>
#include <string>

namespace L3 {

  enum class Op {
    ADD,
    SUB,
    MUL,
    AND,
    SHL,
    SHR
  };

  enum class Cmp {
    LEQ,
    GEQ,
    LT,
    GT,
    EQ
  };

  class Item {
    public:
      virtual ~Item() = default;
  };

  class Number : public Item {
    public:
      Number(int64_t value);

    private:
      int64_t value;
  };

  class Variable : public Item {
    public:
      Variable(std::string name);
    
    private:
      std::string name;
  };

  class Label : public Item {
    public:
      Label(std::string lbl);

    private:
      std::string lbl;
  };

  class ItemList {
    public:
      ItemList(std::vector<Item *> items);
      ~ItemList();

    private:
      std::vector<Item *> items;
  };

  class Instruction {
    public:
      virtual ~Instruction() = default;
  };

  class Instruction_return : public Instruction {

  };

  class Instruction_return_val : public Instruction {
    public:
      Instruction_return_val(Item *val);
      ~Instruction_return_val();

    private:
      Item *val;
  };

  class Instruction_assignment : public Instruction {
    public:
      Instruction_assignment(Item *dst, Item *src);
      ~Instruction_assignment();

    private:
      Item *dst;
      Item *src;
  };

  class Instruction_op_assign : public Instruction {
    public:
      Instruction_op_assign(Item *dst, Item *lhs, Op op, Item *rhs);
      ~Instruction_op_assign();

    private:
      Item *dst;
      Item *lhs;
      Op op;
      Item *rhs;
  };

  class Instruction_cmp_assign : public Instruction {
    public:
      Instruction_cmp_assign(Item *dst, Item *lhs, Cmp cmp, Item *rhs);
      ~Instruction_cmp_assign();

    private:
      Item *dst;
      Item *lhs;
      Cmp cmp;
      Item *rhs;
  };

  class Function {
    public:
      std::string name;
      std::vector<Variable *> arguments;
      std::vector<Instruction *> instructions;

      ~Function();
  };

  class Program {
    public:
      std::vector<Function *> functions;

      std::vector<Item *> parsedItems;
      std::vector<ItemList *> parsedItemLists;
      std::vector<Op> parsedOps;
      std::vector<Cmp> parsedCmps;

      ~Program();
  };
}
#pragma once

#include <vector>
#include <string>
#include <ostream>
#include <set>
#include <map>
#include <iostream>

namespace L2 {

  enum RegisterID {
    rax, rbx, rcx, rdx,
    rsi, rdi, rbp, rsp,
    r8,  r9,  r10, r11,
    r12, r13, r14, r15
  };

  enum ArithOp {
    ADD, 
    SUB, 
    MUL,
    AND
  };

  enum CompareOp {
    LT,
    LEQ,
    EQ
  };

  enum ShiftOp {
    SHL,
    SHR
  };

  struct Context {
    uint64_t locals;
    std::map<std::string, std::string> colors;
    std::set<std::string> calleeSaves;
  };

  struct SpillContext {
    std::string var;
    std::string prefix;
    uint64_t count;
    uint64_t stackAddr;
  };

  class Item {
    public:
      virtual ~Item() = default;
      virtual void fillSet(std::set<std::string> &set) const = 0;
      virtual std::string format() const = 0;
      virtual std::string formatFinal(Context c) const = 0;
      virtual std::string formatSpill(std::string var) const { return ""; };
  };

  class Register : public Item {
    public:
      virtual void fillSet(std::set<std::string> &set) const override;
      Register (RegisterID r);
      virtual std::string format() const override;
      virtual std::string formatFinal(Context c) const override;

    private:
      RegisterID ID;
  };

  class Number : public Item {
    public:
      virtual void fillSet(std::set<std::string> &set) const override;
      Number (int64_t n);
      virtual std::string format() const override;
      virtual std::string formatFinal(Context c) const override;
      
      int64_t get_n();

    private:
      int64_t n;
  };

  class Label : public Item {
    public:
      virtual void fillSet(std::set<std::string> &set) const override;
      Label (std::string v);
      virtual std::string format() const override;
      virtual std::string formatFinal(Context c) const override;

      std::string get() const {return value;}

    private:
      std::string value;
  };

  class Variable : public Item {
    public:
      virtual void fillSet(std::set<std::string> &set) const override;
      std::string getName() { return name; }
      Variable(std::string name);
      virtual std::string format() const override;
      virtual std::string formatFinal(Context c) const override;

    private:
      std::string name;
  };

  class Memory : public Item {
    public:
      virtual void fillSet(std::set<std::string> &set) const override;
      Memory (Item *base, int64_t offset);
      ~Memory() override;
      virtual std::string format() const override;
      virtual std::string formatFinal(Context c) const override;
      virtual std::string formatSpill(std::string var) const override;

    private:
      Item *base;
      int64_t offset;
  };

  /*
   * Instruction interface.
   */
  class Instruction{
    public:
      virtual ~Instruction() = default;
      virtual std::set<std::string> getGEN() const = 0;
      virtual std::set<std::string> getKILL() const = 0;

      virtual void generate(std::ostream &out, Context &c) const = 0;
      virtual void genSpill(std::ostream &out, SpillContext &c) const = 0;
  };

  /*
   * Instructions.
   */
  class Instruction_ret : public Instruction{
    public:
      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;


  };

  class Instruction_assignment : public Instruction{
    public:
      Instruction_assignment (Item *dst, Item *src);
      ~Instruction_assignment() override;

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

    private:
      Item *dst;
      Item *src;
  };

  class Instruction_store : public Instruction{
    public:
      Instruction_store (Item *mem, Item *src);
      ~Instruction_store() override;

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

    private:
      Item *mem;
      Item *src;
  };

  class Instruction_load : public Instruction{
    public:
      Instruction_load (Item *dst, Item *mem);
      ~Instruction_load() override;

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

    private:
      Item *dst;
      Item *mem;
  };

  class Instruction_stack_arg : public Instruction {
    public:
      Instruction_stack_arg(Item *dst, uint64_t offset);
      ~Instruction_stack_arg() override;

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

    private:
      Item *dst;
      uint64_t offset;
  };

  class Instruction_arith : public Instruction{
    public:
      Instruction_arith (Item *lhs, ArithOp op, Item *rhs);
      ~Instruction_arith() override;

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

    private:
      Item *lhs;
      ArithOp op;
      Item *rhs;
  };

  class Instruction_inc : public Instruction{
    public:
      Instruction_inc (Item *val);
      ~Instruction_inc() override;

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

    private:
      Item *val;
  };

  class Instruction_dec : public Instruction{
    public:

      Instruction_dec (Item *val);
      ~Instruction_dec() override;

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

    private:
      Item *val;
  };

  class Instruction_mem_rhs : public Instruction{
    public:

      Instruction_mem_rhs (Item *lhs, ArithOp op, Item *mem);
      ~Instruction_mem_rhs() override;

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

    private:
      Item *lhs;
      ArithOp op;
      Item *mem;
  };

  class Instruction_mem_lhs : public Instruction{
    public:

      Instruction_mem_lhs (Item *mem, ArithOp op, Item *rhs);
      ~Instruction_mem_lhs() override;

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

    private:
      Item *mem;
      ArithOp op;
      Item *rhs;
  };

  class Instruction_compare_assign : public Instruction{
    public:

      Instruction_compare_assign (Item *dst, Item *lhs, CompareOp op, Item *rhs);
      ~Instruction_compare_assign() override;

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

    private:
      Item *dst;
      Item *lhs;
      CompareOp op;
      Item *rhs;
  };

  class Instruction_label : public Instruction{
    public:

      Instruction_label (Item *lbl);
      ~Instruction_label() override;

      Item *getLabel() const {return lbl;}

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

    private:
      Item *lbl;
  };

  class Instruction_goto : public Instruction{
    public:

      Instruction_goto (Item *lbl);
      ~Instruction_goto() override;

      Item *getLabel() const {return lbl;}

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

    private:
      Item* lbl;
  };

  class Instruction_cjump : public Instruction{
    public:

      Instruction_cjump (Item *lhs, CompareOp op, Item *rhs, Item *lbl);
      ~Instruction_cjump() override;

      Item *getLabel() const {return lbl;}

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

    private:
      Item *lhs;
      CompareOp op;
      Item *rhs;
      Item *lbl;
  };

  class Instruction_shift : public Instruction{
    public:

      Instruction_shift (Item *val, ShiftOp op, Item *src);
      ~Instruction_shift() override;

      Item *getSrc() { return src; }

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

    private:
      Item *val;
      ShiftOp op;
      Item *src;
  };

  class Instruction_lea : public Instruction{
    public:

      Instruction_lea (Item *dst, Item *src1, Item *src2, uint64_t offset);
      ~Instruction_lea() override;

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

    private:
      Item *dst;
      Item *src1;
      Item *src2;
      uint64_t offset;
  };

  class Instruction_call : public Instruction{
    public:

      Instruction_call (Item *func, uint64_t args);
      ~Instruction_call() override;

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

    private:
      Item *func;
      uint64_t args;
  };

  class Instruction_call_print : public Instruction{
    public:

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

  };

  class Instruction_call_input : public Instruction{
    public:

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

  };

  class Instruction_call_allocate : public Instruction{
    public:

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

  };

  class Instruction_call_tuple_error : public Instruction{
    public:

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

  };

  class Instruction_call_tensor_error : public Instruction{
    public:

      Instruction_call_tensor_error(uint64_t args);

      virtual std::set<std::string> getGEN() const override;
      virtual std::set<std::string> getKILL() const override;

      virtual void generate(std::ostream &out, Context &c) const override;
      virtual void genSpill(std::ostream &out, SpillContext &c) const override;

    private:
      uint64_t args;
  };


  struct CFGNode {
    Instruction *instr;
    std::vector<CFGNode *> successors;
  };

  struct ParseState {
    public:
      std::vector<Item*> parsed_items;
      std::vector<ArithOp> parsed_arith_ops;
      std::vector<CompareOp> parsed_compare_ops;
      std::vector<ShiftOp> parsed_shift_ops;
  };

  /*
   * Function.
   */
  class Function : public ParseState {
    public:
      std::string name;
      int64_t arguments;
      int64_t locals = 0;
      std::vector<Instruction *> instructions;

      std::vector<std::set<std::string>> GEN;
      std::vector<std::set<std::string>> KILL;
      std::vector<std::set<std::string>> IN;
      std::vector<std::set<std::string>> OUT;

      std::map<std::string, std::set<std::string>> graph;
      std::map<std::string, std::string> colors;
      std::set<std::string> calleeSaves;
      std::set<std::string> shiftVars;

      ~Function();
  };

  class Program : public ParseState{
    public:
      std::string entryPointLabel;
      std::vector<Function *> functions;

      ~Program();
  };

}

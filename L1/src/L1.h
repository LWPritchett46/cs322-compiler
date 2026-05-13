#pragma once

#include <vector>
#include <string>
#include <ostream>

namespace L1 {

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
    uint64_t args;
    uint64_t locals;
  };

  class Item {
    public:
      virtual ~Item() = default;
      virtual std::string format() const = 0;
      virtual std::string format8() const = 0; // special format for 8 bit registers
  };

  class Register : public Item {
    public:
      virtual std::string format() const override;
      virtual std::string format8() const override;
      Register (RegisterID r);

    private:
      RegisterID ID;
  };

  class Number : public Item {
    public:
      virtual std::string format() const override;
      virtual std::string format8() const override { return ""; }
      Number (int64_t n);
      
      int64_t get_n();

    private:
      int64_t n;
  };

  class Label : public Item {
    public:
      virtual std::string format() const override;
      virtual std::string format8() const override { return ""; }
      Label (std::string v);

    private:
      std::string value;
  };

  class Memory : public Item {
    public:
      virtual std::string format() const override;
      virtual std::string format8() const override { return ""; }
      Memory (Item *base, int64_t offset);
      ~Memory() override;

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
      virtual void generate(std::ostream& out, Context c) const = 0;
  };

  /*
   * Instructions.
   */
  class Instruction_ret : public Instruction{
    public:
      virtual void generate(std::ostream& out, Context c) const override;

  };

  class Instruction_assignment : public Instruction{
    public:
      virtual void generate(std::ostream& out, Context c) const override;
      Instruction_assignment (Item *dst, Item *src);
      ~Instruction_assignment() override;

    private:
      Item *dst;
      Item *src;
  };

  class Instruction_store : public Instruction{
    public:
      virtual void generate(std::ostream& out, Context c) const override;
      Instruction_store (Item *mem, Item *src);
      ~Instruction_store() override;

    private:
      Item *mem;
      Item *src;
  };

  class Instruction_load : public Instruction{
    public:
      virtual void generate(std::ostream& out, Context c) const override;
      Instruction_load (Item *dst, Item *mem);
      ~Instruction_load() override;

    private:
      Item *dst;
      Item *mem;
  };

  class Instruction_arith : public Instruction{
    public:
      virtual void generate(std::ostream& out, Context c) const override;
      Instruction_arith (Item *lhs, ArithOp op, Item *rhs);
      ~Instruction_arith() override;

    private:
      Item *lhs;
      ArithOp op;
      Item *rhs;
  };

  class Instruction_inc : public Instruction{
    public:
      virtual void generate(std::ostream& out, Context c) const override;
      Instruction_inc (Item *val);
      ~Instruction_inc() override;

    private:
      Item *val;
  };

  class Instruction_dec : public Instruction{
    public:
      virtual void generate(std::ostream& out, Context c) const override;
      Instruction_dec (Item *val);
      ~Instruction_dec() override;

    private:
      Item *val;
  };

  class Instruction_mem_rhs : public Instruction{
    public:
      virtual void generate(std::ostream& out, Context c) const override;
      Instruction_mem_rhs (Item *lhs, ArithOp op, Item *mem);
      ~Instruction_mem_rhs() override;

    private:
      Item *lhs;
      ArithOp op;
      Item *mem;
  };

  class Instruction_mem_lhs : public Instruction{
    public:
      virtual void generate(std::ostream& out, Context c) const override;
      Instruction_mem_lhs (Item *mem, ArithOp op, Item *rhs);
      ~Instruction_mem_lhs() override;

    private:
      Item *mem;
      ArithOp op;
      Item *rhs;
  };

  class Instruction_compare_assign : public Instruction{
    public:
      virtual void generate(std::ostream& out, Context c) const override;
      Instruction_compare_assign (Item *dst, Item *lhs, CompareOp op, Item *rhs);
      ~Instruction_compare_assign() override;

    private:
      Item *dst;
      Item *lhs;
      CompareOp op;
      Item *rhs;
  };

  class Instruction_label : public Instruction{
    public:
      virtual void generate(std::ostream &out, Context c) const override;
      Instruction_label (Item *lbl);
      ~Instruction_label() override;

    private:
      Item *lbl;
  };

  class Instruction_goto : public Instruction{
    public:
      virtual void generate(std::ostream &out, Context c) const override;
      Instruction_goto (Item *lbl);
      ~Instruction_goto() override;

    private:
      Item* lbl;
  };

  class Instruction_cjump : public Instruction{
    public:
      virtual void generate(std::ostream& out, Context c) const override;
      Instruction_cjump (Item *lhs, CompareOp op, Item *rhs, Item *lbl);
      ~Instruction_cjump() override;

    private:
      Item *lhs;
      CompareOp op;
      Item *rhs;
      Item *lbl;
  };

  class Instruction_shift : public Instruction{
    public:
      virtual void generate(std::ostream &out, Context c) const override;
      Instruction_shift (Item *val, ShiftOp op, Item *src);
      ~Instruction_shift() override;

    private:
      Item *val;
      ShiftOp op;
      Item *src;
  };

  class Instruction_lea : public Instruction{
    public:
      virtual void generate(std::ostream& out, Context c) const override;
      Instruction_lea (Item *dst, Item *src1, Item *src2, uint64_t offset);
      ~Instruction_lea() override;

    private:
      Item *dst;
      Item *src1;
      Item *src2;
      uint64_t offset;
  };

  class Instruction_call : public Instruction{
    public:
      virtual void generate(std::ostream& out, Context c) const override;
      Instruction_call (Item *func, uint64_t args);
      ~Instruction_call() override;

    private:
      Item *func;
      uint64_t args;
  };

  class Instruction_call_print : public Instruction{
    public:
      virtual void generate(std::ostream& out, Context c) const override;
  };

  class Instruction_call_input : public Instruction{
    public:
      virtual void generate(std::ostream& out, Context c) const override;
  };

  class Instruction_call_allocate : public Instruction{
    public:
      virtual void generate(std::ostream& out, Context c) const override;
  };

  class Instruction_call_tuple_error : public Instruction{
    public:
      virtual void generate(std::ostream& out, Context c) const override;
  };

  class Instruction_call_tensor_error : public Instruction{
    public:
      virtual void generate(std::ostream& out, Context c) const override;
      Instruction_call_tensor_error(uint64_t args);

    private:
      uint64_t args;
  };

  /*
   * Function.
   */
  class Function{
    public:
      std::string name;
      int64_t arguments;
      int64_t locals;
      std::vector<Instruction *> instructions;

      ~Function();
  };

  class Program{
    public:
      std::string entryPointLabel;
      std::vector<Function *> functions;

      std::vector<Item *> parsed_items;
      std::vector<ArithOp> parsed_arith_ops;
      std::vector<CompareOp> parsed_compare_ops;
      std::vector<ShiftOp> parsed_shift_ops;

      ~Program();
  };

}

#pragma once

#include <vector>
#include <string>
#include <map>

namespace L3 {

  class TreeNode;

  enum class NodeKind {
    VAR,
    NUM,
    LBL,

    OP,

    CMP,

    LOAD,
    STORE,

    MOVE,

    RETURN,
    BRANCH,
    CJUMP
  };

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
      virtual TreeNode* makeNode() const = 0;
  };

  class Number : public Item {
    public:
      Number(int64_t value);
      virtual TreeNode* makeNode() const override;

    private:
      int64_t value;
  };

  class Variable : public Item {
    public:
      Variable(std::string name);
      virtual TreeNode* makeNode() const override;
    
    private:
      std::string name;
  };

  class Label : public Item {
    public:
      Label(std::string name);
      std::string getName();
      void setName(std::string newName);
      virtual TreeNode* makeNode() const override;

    private:
      std::string name;
  };

  class ItemList {
    public:
      ItemList(std::vector<Item *> items);
      ~ItemList();

    private:
      std::vector<Item *> items;
  };


  class TreeNode {
    public:
      NodeKind kind;

      std::vector<TreeNode*> children;

      const Item *item = nullptr;

      Op op;
      Cmp cmp;
  };


  class Instruction {
    public:
      virtual ~Instruction() = default;
      virtual void rewriteLabels(std::map<std::string, std::string> labelMap);
      virtual TreeNode* generateTree() const = 0;
  };

  class Instruction_return : public Instruction {
    public:
      virtual TreeNode* generateTree() const override;
  };

  class Instruction_return_val : public Instruction {
    public:
      Instruction_return_val(Item *val);
      ~Instruction_return_val();
      virtual TreeNode* generateTree() const override;

    private:
      Item *val;
  };

  class Instruction_assignment : public Instruction {
    public:
      Instruction_assignment(Item *dst, Item *src);
      ~Instruction_assignment();
      virtual void rewriteLabels(std::map<std::string, std::string> labelMap) override;
      virtual TreeNode* generateTree() const override;

    private:
      Item *dst;
      Item *src;
  };

  class Instruction_op_assign : public Instruction {
    public:
      Instruction_op_assign(Item *dst, Item *lhs, Op op, Item *rhs);
      ~Instruction_op_assign();
      virtual TreeNode* generateTree() const override;

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
      virtual TreeNode* generateTree() const override;

    private:
      Item *dst;
      Item *lhs;
      Cmp cmp;
      Item *rhs;
  };

  class Instruction_load : public Instruction {
    public:
      Instruction_load(Item *dst, Item *ptr);
      ~Instruction_load();
      virtual TreeNode* generateTree() const override;

    private:
      Item *dst;
      Item *ptr;
  };

  class Instruction_store : public Instruction {
    public:
      Instruction_store(Item *ptr, Item *src);
      ~Instruction_store();
      virtual void rewriteLabels(std::map<std::string, std::string> labelMap) override;
      virtual TreeNode* generateTree() const override;

    private:
      Item *ptr;
      Item *src;
  };

  class Instruction_label : public Instruction {
    public:
      Instruction_label(Item *lbl);
      ~Instruction_label();
      Item *getLabel();
      virtual void rewriteLabels(std::map<std::string, std::string> labelMap) override;
      virtual TreeNode* generateTree() const override;

    private:
      Item *lbl;
  };

  class Instruction_branch : public Instruction {
    public:
      Instruction_branch(Item *lbl);
      ~Instruction_branch();
      virtual void rewriteLabels(std::map<std::string, std::string> labelMap) override;
      virtual TreeNode* generateTree() const override;

    private:
      Item *lbl;
  };

  class Instruction_branch_cond : public Instruction {
    public:
      Instruction_branch_cond(Item *cond, Item *lbl);
      ~Instruction_branch_cond();
      virtual void rewriteLabels(std::map<std::string, std::string> labelMap) override;
      virtual TreeNode* generateTree() const override;

    private:
      Item *cond;
      Item *lbl;
  };

  class Instruction_call : public Instruction {
    public:
      Instruction_call(Item *callee, ItemList *args);
      ~Instruction_call();
      virtual TreeNode* generateTree() const override;

    private:
      Item *callee;
      ItemList *args;
  };

  class Instruction_call_assign : public Instruction {
    public:
      Instruction_call_assign(Item *dst, Item *callee, ItemList *args);
      ~Instruction_call_assign();
      virtual TreeNode* generateTree() const override;

    private:
      Item *dst;
      Item *callee;
      ItemList *args;
  };


  class FunctionElement {
    public:
      virtual ~FunctionElement() = default;
  };

  class Context : public FunctionElement {
    public:
      std::vector<Instruction *> instructions;

      std::vector<TreeNode *> forest;
  };

  class SoloInstruction : public FunctionElement {
    public:
      Instruction *inst;
  };

  class Function {
    public:
      std::string name;
      std::vector<Variable *> arguments;
      std::vector<Instruction *> instructions;
      std::vector<FunctionElement *> elements;

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
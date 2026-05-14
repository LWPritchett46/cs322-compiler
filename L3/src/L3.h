#pragma once

namespace L3 {
  class Item {
    public:
      virtual ~Item() = default;
  };

  class Instruction {
    public:
      virtual ~Instruction() = default;
  };

  class Function {
    public:
      std::vector<Instruction *> instructions;

      ~Function();
  };

  class Program {
    public:
      std::vector<Function *> functions;
  }
}
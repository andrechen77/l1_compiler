#pragma once

#include <vector>
#include <string>

namespace L1 {

  enum RegisterID {rdi, rax};

  class Item {
  };

  class Register : public Item {
    public:
      Register (RegisterID r);

    private:
      RegisterID ID;
  };

  /*
   * Instruction interface.
   */
  class Instruction{
  };

  /*
   * Instructions.
   */
  class Instruction_ret : public Instruction{
  };

  class Instruction_assignment : public Instruction{
    public:
      Instruction_assignment (Item *dst, Item *src);

    private:
      Item *s;
      Item *d;
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
  };

  class Program{
    public:
      std::string entryPointLabel;
      std::vector<Function *> functions;
  };

}

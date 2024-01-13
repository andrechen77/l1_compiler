#include "L1.h"

namespace L1 {

Register::Register (RegisterID r)
  : ID {r}{
  return ;
}

Instruction_assignment::Instruction_assignment (Item *dst, Item *src)
  : s { src },
    d { dst } {
  return ;
}

}

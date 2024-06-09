#ifndef NESEMULATOR_INSTRUCTIONS_H
#define NESEMULATOR_INSTRUCTIONS_H

#include <stdlib.h>
#include <memory>
#include "state.h"

class Instruction { ;
public:
    //return # of cycles
    virtual int act(Cpu6502_State &cpu_state);
};

using InstructionPtr = std::shared_ptr<Instruction>;


extern const std::array<InstructionPtr, 256> instructions;

#endif //NESEMULATOR_INSTRUCTIONS_H

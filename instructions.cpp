#include "state.cpp"


class InstructionAction {
    virtual void act(Cpu6502_State state) {};
};

class EndInstruction : public InstructionAction {
    //Special handling by emulator
};

class NoopWait : InstructionAction {
    void act(Cpu6502_State state) override {
        state.getReg().incrPC();
    }
};

class Instruction {
    std::array<InstructionAction, 8> actions = {EndInstruction()};
};

array<Instruction, 128> instruction_ref;



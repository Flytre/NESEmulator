#include "state.cpp"
#include "functional"

using namespace std;

class Instruction {
    //return # of cycles
    virtual int act(Cpu6502_State cpu_state) {
        return 2;
    };
};

using InstructionPtr = shared_ptr<Instruction>;

enum class IndexType {
    X,
    Y
};


class FunctionInstruction : public Instruction {
public:
    FunctionInstruction(function<int(Cpu6502_State)> func)
            : func_(func) {}

    int act(Cpu6502_State cpu_state) override {
        return func_(cpu_state);
    }

private:
    function<int(Cpu6502_State)> func_;
};

uint16_t get_2b_addr(Cpu6502_State cs) {
    uint8_t low = cs.get_instr_byte();
    uint8_t high = cs.get_instr_byte();
    return (static_cast<uint16_t>(high) << 8) | static_cast<uint16_t>(low);
}

//val, page cross
pair<uint8_t, bool> get_x_idx_val(Cpu6502_State cs, uint16_t base_addr) {
    uint16_t addr = base_addr + static_cast<uint16_t>(cs.getReg().getX());
    uint8_t val = cs.get_byte(addr);
    bool p = (base_addr & 0xFF00) != (addr & 0xFF00);
    return {val, p};
}

//val, page cross
pair<uint8_t, bool> get_y_idx_val(Cpu6502_State cs, uint16_t base_addr) {
    uint16_t addr = base_addr + static_cast<uint16_t>(cs.getReg().getY());
    uint8_t val = cs.get_byte(addr);
    bool p = (base_addr & 0xFF00) != (addr & 0xFF00);
    return {val, p};
}

uint16_t get_indirect_addr(Cpu6502_State cs, uint8_t addr_addr) {
    uint8_t addr_addr_2 = addr_addr + 1;
    uint8_t lower = cs.get_byte(addr_addr);
    uint8_t higher = cs.get_byte(addr_addr_2);
    uint16_t addr = static_cast<uint16_t>(lower) | (static_cast<uint16_t>(higher) << 8);
    return addr;
}

uint8_t get_x_idx_zero_page_indirect_val(Cpu6502_State cs) {
    uint8_t addr_addr = cs.get_instr_byte() + cs.getReg().getX();
    return cs.get_byte(get_indirect_addr(cs, addr_addr));
}

pair<uint8_t, bool> get_zero_page_indirect_y_idx_val(Cpu6502_State cs) {
    uint8_t addr_addr = cs.get_instr_byte();
    uint16_t indir_addr = get_indirect_addr(cs, addr_addr);
    return get_y_idx_val(cs, indir_addr);
}

void adc(Cpu6502_State cs, uint8_t val) {
    return cs.getReg().setA(cs.add(val, cs.getReg().getA()));
}

//Fetch is assumed to run before this happens automatically
array<InstructionPtr, 128> instruction_ref() {
    array<InstructionPtr, 128> res{};

    //ADC
    res[0x69] = make_shared<FunctionInstruction>([](Cpu6502_State cs) {
        uint8_t imm = cs.get_instr_byte();
        adc(cs, imm);
        return 2;
    });
    res[0x6D] = make_shared<FunctionInstruction>([](Cpu6502_State cs) {
        uint16_t addr = get_2b_addr(cs);
        adc(cs, cs.get_byte(addr));
        return 4;
    });
    res[0x7D] = make_shared<FunctionInstruction>([](Cpu6502_State cs) {
        auto [val, p] = get_x_idx_val(cs, get_2b_addr(cs));
        adc(cs, val);
        return 4 + p;
    });
    res[0x79] = make_shared<FunctionInstruction>([](Cpu6502_State cs) {
        auto [val, p] = get_y_idx_val(cs, get_2b_addr(cs));
        adc(cs, val);
        return 4 + p;
    });
    res[0x65] = make_shared<FunctionInstruction>([](Cpu6502_State cs) {
        uint8_t addr = cs.get_instr_byte();
        adc(cs, cs.get_byte(addr));
        return 3;
    });
    res[0x75] = make_shared<FunctionInstruction>([](Cpu6502_State cs) {
        auto [val, p] = get_x_idx_val(cs, cs.get_instr_byte());
        adc(cs, val);
        return 4;
    });
    res[0x61] = make_shared<FunctionInstruction>([](Cpu6502_State cs) {
        uint8_t val = get_x_idx_zero_page_indirect_val(cs);
        adc(cs, val);
        return 6;
    });
    res[0x71] = make_shared<FunctionInstruction>([](Cpu6502_State cs) {
        auto [val, p] = get_zero_page_indirect_y_idx_val(cs);
        adc(cs, val);
        return 5 + p;
    });
    return res;
}

const array<InstructionPtr, 128> instructions = instruction_ref();
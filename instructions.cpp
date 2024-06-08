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

class AddrOrVal {
private:
    bool is_addr;
    Addr addr;
    Val val;

public:
    AddrOrVal(bool isAddr, const Addr &addr, const Val &val) : is_addr(isAddr), addr(addr), val(val) {}

public:
    static AddrOrVal create_addr(Addr addr) {
        return AddrOrVal(true, addr, 0);
    }

    static AddrOrVal create_val(Val val) {
        return AddrOrVal(true, 0, val);
    }

    static AddrOrVal create(bool is_address, Addr addr, Val val) {
        return is_address ? create_addr(addr) : create_val(val);
    }

    Addr getAddr() {
        if (!is_addr)
            throw invalid_argument("Not an address");
    }

    Val getVal() {
        if (is_addr)
            throw invalid_argument("Not a value");

    }
};

Addr get_2b_addr(Cpu6502_State cs) {
    Val low = cs.get_instr_byte();
    Val high = cs.get_instr_byte();
    return (static_cast<uint16_t>(high.val) << 8) | static_cast<uint16_t>(low.val);
}

pair<AddrOrVal, bool> x_indexed(Cpu6502_State cs, Addr base_addr, bool return_address) {
    Addr addr = base_addr + Addr(cs.getReg().getX().val);
    bool p = (base_addr.addr & 0xFF00) != (addr.addr & 0xFF00);
    return {AddrOrVal(return_address, addr, cs.get_byte(addr)), p};
}

pair<AddrOrVal, bool> x_indexed(Cpu6502_State cs, ZeroPageAddr base_addr, bool return_address) {
    ZeroPageAddr addr = base_addr + ZeroPageAddr(cs.getReg().getX().val);
    bool p = (base_addr.addr & 0xFF00) != (addr.addr & 0xFF00);
    return {AddrOrVal(return_address, Addr(addr.addr), cs.get_byte(addr)), p};
}

pair<AddrOrVal, bool> y_indexed(Cpu6502_State cs, Addr base_addr, bool return_address) {
    Addr addr = base_addr + Addr(cs.getReg().getY().val);
    bool p = (base_addr.addr & 0xFF00) != (addr.addr & 0xFF00);
    return {AddrOrVal(return_address, addr, cs.get_byte(addr)), p};
}

pair<AddrOrVal, bool> y_indexed(Cpu6502_State cs, ZeroPageAddr base_addr, bool return_address) {
    ZeroPageAddr addr = base_addr + ZeroPageAddr(cs.getReg().getY().val);
    bool p = (base_addr.addr & 0xFF00) != (addr.addr & 0xFF00);
    return {AddrOrVal(return_address, Addr(addr.addr), cs.get_byte(addr)), p};
}

Addr indirect_addr(Cpu6502_State cs, ZeroPageAddr addr_addr) {
    ZeroPageAddr addr_addr_2 = addr_addr + ZeroPageAddr(1);
    Val lower = cs.get_byte(addr_addr);
    Val higher = cs.get_byte(addr_addr_2);
    Addr addr = Addr(lower.val) | (Addr(higher.val) << 8);
    return addr;
}

AddrOrVal x_indexed_zero_page_indirect(Cpu6502_State cs, bool return_address) {
    ZeroPageAddr addr_addr = cs.get_instr_byte() + cs.getReg().getX();
    Addr addr = indirect_addr(cs, addr_addr);
    return AddrOrVal::create(return_address, addr, cs.get_byte(addr));
}


pair<AddrOrVal, bool> zero_page_indirect_y_indexed(Cpu6502_State cs, bool return_address) {
    ZeroPageAddr addr_addr = cs.get_instr_byte();
    Addr indir_addr = indirect_addr(cs, addr_addr);
    return y_indexed(cs, indir_addr, return_address);
}

void instr_adc(Cpu6502_State cs, AddrOrVal arg) {
    Val val = arg.getVal();
    cs.getReg().setA(cs.add(val, cs.getReg().getA()));
}

void instr_and(Cpu6502_State cs, AddrOrVal arg) {
    Val val = arg.getVal();
    Val res = val & cs.getReg().getA();
    cs.getReg().set_flag(FlagPositions::ZERO, res == 0);
    cs.getReg().set_flag(FlagPositions::NEG, res.val & 0x80);
    cs.getReg().setA(res);
}

void instr_lda(Cpu6502_State cs, AddrOrVal arg) {
    Val val = arg.getVal();
    cs.getReg().set_flag(FlagPositions::ZERO, val == 0);
    cs.getReg().set_flag(FlagPositions::NEG, val.val & 0x80);
    cs.getReg().setA(val);
}

void instr_sta(Cpu6502_State cs, AddrOrVal arg) {
    Addr addr = arg.getAddr();
    cs.set_byte(addr, cs.getReg().getA());
}

void instr_eor(Cpu6502_State cs, AddrOrVal arg) {
    Val val = arg.getVal();
    Val res = val ^ cs.getReg().getA();
    cs.getReg().set_flag(FlagPositions::ZERO, res == 0);
    cs.getReg().set_flag(FlagPositions::NEG, res.val & 0x80);
    cs.getReg().setA(res);
}

void instr_ora(Cpu6502_State cs, AddrOrVal arg) {
    Val val = arg.getVal();
    Val res = val | cs.getReg().getA();
    cs.getReg().set_flag(FlagPositions::ZERO, res == 0);
    cs.getReg().set_flag(FlagPositions::NEG, res.val & 0x80);
    cs.getReg().setA(res);
}

void instr_cmp(Cpu6502_State cs, AddrOrVal arg) {
    Val val = arg.getVal();
    Val res = (cs.getReg().getA() - val).val;
    cs.getReg().set_flag(FlagPositions::CARRY, val <= cs.getReg().getA());
    cs.getReg().set_flag(FlagPositions::ZERO, res == 0);
    cs.getReg().set_flag(FlagPositions::NEG, res.val & 0x80);
}

void instr_sbc(Cpu6502_State cs, AddrOrVal arg) {
    Val val = arg.getVal();
    Val A = cs.getReg().getA();
    Val borrow = cs.getReg().flag_set(FlagPositions::CARRY) ? 0 : 1;
    Val result = A - val - borrow;

    bool overflow = ((A.val ^ result.val) & 0x80) && ((A.val ^ val.val) & 0x80);
    bool carry = A >= val + borrow;

    cs.getReg().set_flag(FlagPositions::CARRY, carry);
    cs.getReg().set_flag(FlagPositions::ZERO, result == 0);
    cs.getReg().set_flag(FlagPositions::NEG, result.val & 0x80);
    cs.getReg().set_flag(FlagPositions::OVF, overflow);

    cs.getReg().setA(result);
}

void create_acc_suite(array<InstructionPtr, 128> res, int base_addr,
                      const function<void(Cpu6502_State, AddrOrVal arg)> &func, bool return_addr) {
    if (!return_addr)
        res[base_addr + 0x09] = make_shared<FunctionInstruction>([&func](Cpu6502_State cs) {
            Val imm = cs.get_instr_byte();
            func(cs, AddrOrVal::create_val(imm));
            return 2;
        });
    res[base_addr + 0x0D] = make_shared<FunctionInstruction>([&func, return_addr](Cpu6502_State cs) {
        Addr addr = get_2b_addr(cs);
        func(cs, AddrOrVal::create(return_addr, addr, cs.get_byte(addr)));
        return 4;
    });
    res[base_addr + 0x1D] = make_shared<FunctionInstruction>([&func, return_addr](Cpu6502_State cs) {
        auto [val, p] = x_indexed(cs, get_2b_addr(cs), return_addr);
        func(cs, val);
        return 4 + p;
    });
    res[base_addr + 0x19] = make_shared<FunctionInstruction>([&func, return_addr](Cpu6502_State cs) {
        auto [val, p] = y_indexed(cs, get_2b_addr(cs), return_addr);
        func(cs, val);
        return 4 + p;
    });
    res[base_addr + 0x05] = make_shared<FunctionInstruction>([&func, return_addr](Cpu6502_State cs) {
        ZeroPageAddr addr = cs.get_instr_byte();
        func(cs, AddrOrVal::create(return_addr, Addr(addr.addr), cs.get_byte(addr)));
        return 3;
    });
    res[base_addr + 0x15] = make_shared<FunctionInstruction>([&func, return_addr](Cpu6502_State cs) {
        auto [val, p] = x_indexed(cs, cs.get_instr_byte(), return_addr);
        func(cs, val);
        return 4;
    });
    res[base_addr + 0x01] = make_shared<FunctionInstruction>([&func, return_addr](Cpu6502_State cs) {
        func(cs, x_indexed_zero_page_indirect(cs, return_addr));
        return 6;
    });
    res[0x11] = make_shared<FunctionInstruction>([&func, return_addr](Cpu6502_State cs) {
        auto [val, p] = zero_page_indirect_y_indexed(cs, return_addr);
        func(cs, val);
        return 5 + p;
    });
}

//Fetch is assumed to run before this happens automatically
array<InstructionPtr, 128> instruction_ref() {
    array<InstructionPtr, 128> res{};

    create_acc_suite(res, 0x00, instr_ora, false);
    create_acc_suite(res, 0x20, instr_adc, false);
    create_acc_suite(res, 0x40, instr_eor, false);
    create_acc_suite(res, 0x60, instr_and, false);
    create_acc_suite(res, 0x80, instr_sta, true);
    create_acc_suite(res, 0xA0, instr_lda, false);
    create_acc_suite(res, 0xC0, instr_cmp, false);
    create_acc_suite(res, 0xE0, instr_sbc, false);

    return res;
}

const array<InstructionPtr, 128> instructions = instruction_ref();
#pragma once

#include <utility>

#include "state.h"
#include "functional"
#include "basics.h"
#include "instructions.h"

using namespace std;


int Instruction::act(Cpu6502_State &cpu_state) {
    return 2;
}

using InstructionPtr = shared_ptr<Instruction>;


class FunctionInstruction : public Instruction {
public:
    explicit FunctionInstruction(function<int(Cpu6502_State &)> func)
            : func_(std::move(func)) {}

    int act(Cpu6502_State &cpu_state) override {
        return func_(cpu_state);
    }

private:
    function<int(Cpu6502_State &)> func_;
};

Addr get_2b_addr(Cpu6502_State &cs) {
    Val low = cs.get_instr_byte();
    Val high = cs.get_instr_byte();
    return Addr((static_cast<uint16_t>(high.val) << 8) | static_cast<uint16_t>(low.val));
}

pair<AddrOrVal, bool> x_indexed(Cpu6502_State &cs, Addr base_addr, bool return_address) {
    Addr addr = base_addr + Addr(cs.reg().getX().val);
    bool p = (base_addr.addr & 0xFF00) != (addr.addr & 0xFF00);
    return {AddrOrVal(return_address, addr, cs.get_byte(addr)), p};
}

pair<AddrOrVal, bool> x_indexed_zero_page(Cpu6502_State &cs, ZeroPageAddr base_addr, bool return_address) {
    ZeroPageAddr addr = base_addr + ZeroPageAddr(cs.reg().getX());
    bool p = (base_addr.addr & 0xFF00) != (addr.addr & 0xFF00);
    return {AddrOrVal(return_address, Addr(addr.addr), cs.get_byte(addr)), p};
}

pair<AddrOrVal, bool> y_indexed(Cpu6502_State &cs, Addr base_addr, bool return_address) {
    Addr addr = base_addr + Addr(cs.reg().getY().val);
    bool p = (base_addr.addr & 0xFF00) != (addr.addr & 0xFF00);
    return {AddrOrVal(return_address, addr, cs.get_byte(addr)), p};
}

pair<AddrOrVal, bool> y_indexed_zero_page(Cpu6502_State &cs, ZeroPageAddr base_addr, bool return_address) {
    ZeroPageAddr addr = base_addr + ZeroPageAddr(cs.reg().getY());
    bool p = (base_addr.addr & 0xFF00) != (addr.addr & 0xFF00);
    return {AddrOrVal(return_address, Addr(addr.addr), cs.get_byte(addr)), p};
}


Addr indirect_addr(Cpu6502_State &cs, ZeroPageAddr addr_addr) {
    ZeroPageAddr addr_addr_2 = addr_addr + ZeroPageAddr(Val(1));
    Val lower = cs.get_byte(addr_addr);
    Val higher = cs.get_byte(addr_addr_2);
    Addr addr = Addr(lower.val) | (Addr(higher.val) << 8);
    return addr;
}

//ONLY FOR JUMPS
Addr indirect_addr_jump(Cpu6502_State &cs, Addr addr_addr) {
    Addr addr_addr_2 = addr_addr + Addr(1);
    //hardware issue on the mos 6502
    addr_addr_2 = Addr((addr_addr_2.addr & 0xFF) | (addr_addr.addr & 0xFF00));
    Val lower = cs.get_byte(addr_addr);
    Val higher = cs.get_byte(addr_addr_2);
    Addr addr = Addr(lower.val) | (Addr(higher.val) << 8);
    return addr;
}

AddrOrVal x_indexed_zero_page_indirect(Cpu6502_State &cs, bool return_address) {
    ZeroPageAddr addr_addr = ZeroPageAddr(cs.get_instr_byte() + cs.reg().getX());
    Addr addr = indirect_addr(cs, addr_addr);
    return AddrOrVal::create(return_address, addr, cs.get_byte(addr));
}


pair<AddrOrVal, bool> zero_page_indirect_y_indexed(Cpu6502_State &cs, bool return_address) {
    auto addr_addr = ZeroPageAddr(cs.get_instr_byte());
    Addr indir_addr = indirect_addr(cs, addr_addr);
    return y_indexed(cs, indir_addr, return_address);
}

void instr_adc(Cpu6502_State &cs, AddrOrVal arg) {
    Val val = arg.getVal();
    cs.reg().setA(cs.add(val, cs.reg().getA()));
}

void instr_and(Cpu6502_State &cs, AddrOrVal arg) {
    Val val = arg.getVal();
    Val res = val & cs.reg().getA();
    cs.reg().set_flag(FlagPositions::ZERO, res.val == 0);
    cs.reg().set_flag(FlagPositions::NEG, res.val & 0x80);
    cs.reg().setA(res);
}

void instr_lda(Cpu6502_State &cs, AddrOrVal arg) {
    Val val = arg.getVal();
    cs.reg().set_flag(FlagPositions::ZERO, val.val == 0);
    cs.reg().set_flag(FlagPositions::NEG, val.val & 0x80);
    cs.reg().setA(val);
}

void instr_sta(Cpu6502_State &cs, AddrOrVal arg) {
    Addr addr = arg.getAddr();
    cs.set_byte(addr, cs.reg().getA());
}

void instr_eor(Cpu6502_State &cs, AddrOrVal arg) {
    Val val = arg.getVal();
    Val res = val ^ cs.reg().getA();
    cs.reg().set_flag(FlagPositions::ZERO, res.val == 0);
    cs.reg().set_flag(FlagPositions::NEG, res.val & 0x80);
    cs.reg().setA(res);
}

void instr_ora(Cpu6502_State &cs, AddrOrVal arg) {
    Val val = arg.getVal();
    Val res = val | cs.reg().getA();
    cs.reg().set_flag(FlagPositions::ZERO, res.val == 0);
    cs.reg().set_flag(FlagPositions::NEG, res.val & 0x80);
    cs.reg().setA(res);
}

void instr_cmp(Cpu6502_State &cs, AddrOrVal arg) {
    Val val = arg.getVal();
    Val res = cs.reg().getA() - val;
    cs.reg().set_flag(FlagPositions::CARRY, val <= cs.reg().getA());
    cs.reg().set_flag(FlagPositions::ZERO, res.val == 0);
    cs.reg().set_flag(FlagPositions::NEG, res.val & 0x80);
}

void instr_sbc(Cpu6502_State &cs, AddrOrVal arg) {
    Val val = arg.getVal();
    Val A = cs.reg().getA();
    Val borrow = cs.reg().get_flag(FlagPositions::CARRY) ? Val(0) : Val(1);
    Val result = A - val - borrow;

    bool overflow = ((A.val ^ result.val) & 0x80) && ((A.val ^ val.val) & 0x80);
    bool carry = A >= val + borrow;

    cs.reg().set_flag(FlagPositions::CARRY, carry);
    cs.reg().set_flag(FlagPositions::ZERO, result.val == 0);
    cs.reg().set_flag(FlagPositions::NEG, result.val & 0x80);
    cs.reg().set_flag(FlagPositions::OVF, overflow);

    cs.reg().setA(result);
}

void instr_asl(Cpu6502_State &cs, ValReference ref) {
    Val current = ref.get();
    cs.reg().set_flag(FlagPositions::CARRY, current.val & 0x80);
    Val result = current << 1;
    cs.reg().set_flag(FlagPositions::ZERO, result.val == 0);
    cs.reg().set_flag(FlagPositions::NEG, result.val & 0x80);
    ref.set(result);
}

void instr_lsr(Cpu6502_State &cs, ValReference ref) {
    Val current = ref.get();
    Val result = current >> 1;
    cs.reg().set_flag(FlagPositions::CARRY, current.val & 1);
    cs.reg().set_flag(FlagPositions::NEG, false);
    cs.reg().set_flag(FlagPositions::ZERO, result.val == 0);
    ref.set(result);
}

void instr_rol(Cpu6502_State &cs, ValReference ref) {
    Val current = ref.get();
    bool carry_in = cs.reg().get_flag(FlagPositions::CARRY);
    bool bit7 = (current.val & 0x80) != 0;

    Val result = Val((current.val << 1) | (carry_in ? 1 : 0));

    cs.reg().set_flag(FlagPositions::CARRY, bit7);
    cs.reg().set_flag(FlagPositions::NEG, (result.val & 0x80) != 0);
    cs.reg().set_flag(FlagPositions::ZERO, result.val == 0);

    ref.set(result);
}

void instr_ror(Cpu6502_State &cs, ValReference ref) {
    Val current = ref.get();
    bool carry_in = cs.reg().get_flag(FlagPositions::CARRY);
    bool bit0 = (current.val & 0x01) != 0;  // Check if bit 0 is set

    Val result = Val((current.val >> 1) | (carry_in ? 0x80 : 0));

    cs.reg().set_flag(FlagPositions::CARRY, bit0);
    cs.reg().set_flag(FlagPositions::NEG, carry_in);
    cs.reg().set_flag(FlagPositions::ZERO, result.val == 0);

    ref.set(result);
}

void instr_dec(Cpu6502_State &cs, ValReference ref) {
    Val current = ref.get();
    Val result = current - Val(1);
    cs.reg().set_flag(FlagPositions::NEG, result.val & 0x80);
    cs.reg().set_flag(FlagPositions::ZERO, result.val == 0);
    ref.set(result);
}

void instr_inc(Cpu6502_State &cs, ValReference ref) {
    Val current = ref.get();
    Val result = current + Val(1);
    cs.reg().set_flag(FlagPositions::NEG, result.val & 0x80);
    cs.reg().set_flag(FlagPositions::ZERO, result.val == 0);
    ref.set(result);
}

void instr_ldx(Cpu6502_State &cs, Val val) {
    cs.reg().setX(val);
    cs.reg().set_flag(FlagPositions::NEG, val.val & 0x80);
    cs.reg().set_flag(FlagPositions::ZERO, val.val == 0);
}

void instr_ldy(Cpu6502_State &cs, Val val) {
    cs.reg().setY(val);
    cs.reg().set_flag(FlagPositions::NEG, val.val & 0x80);
    cs.reg().set_flag(FlagPositions::ZERO, val.val == 0);
}

void instr_stx(Cpu6502_State &cs, Addr addr) {
    cs.set_byte(addr, cs.reg().getX());
}

void instr_sty(Cpu6502_State &cs, Addr addr) {
    cs.set_byte(addr, cs.reg().getY());
}

void instr_bit(Cpu6502_State &cs, Val memory_value) {
    Val accumulator = cs.reg().getA();
    Val result = accumulator & memory_value;
    cs.reg().set_flag(FlagPositions::ZERO, result.val == 0);
    cs.reg().set_flag(FlagPositions::NEG, memory_value.val & 0x80);
    cs.reg().set_flag(FlagPositions::OVF, memory_value.val & 0x40);
}

void instr_cpx(Cpu6502_State &cs, Val other) {
    Val result = cs.reg().getX() - other;
    cs.reg().set_flag(FlagPositions::ZERO, result.val == 0);
    cs.reg().set_flag(FlagPositions::CARRY, cs.reg().getX() >= other);
    cs.reg().set_flag(FlagPositions::NEG, result.val & 0x80);
}

void instr_cpy(Cpu6502_State &cs, Val other) {
    Val result = cs.reg().getY() - other;
    cs.reg().set_flag(FlagPositions::ZERO, result.val == 0);
    cs.reg().set_flag(FlagPositions::CARRY, cs.reg().getY() >= other);
    cs.reg().set_flag(FlagPositions::NEG, result.val & 0x80);
}

int branch(Cpu6502_State &cs, const function<bool(Cpu6502_State &)> &cond) {
    int8_t offset = static_cast<int8_t>(cs.get_instr_byte().val);
    if (cond(cs)) {
        uint16_t new_pc_addr = static_cast<uint16_t>(cs.reg().getPC().addr + offset);
        bool page_crossed = (cs.reg().getPC().addr & 0xFF00) != (new_pc_addr & 0xFF00);
        cs.reg().setPC(Addr(new_pc_addr));
        return 3 + (page_crossed ? 1 : 0);
    }
    return 2;
}


ValReference acc_ref(Cpu6502_State &cs) {
    return {
            [&cs]() { return cs.reg().getA(); },
            [&cs](Val x) { return cs.reg().setA(x); }
    };
}

ValReference mem_ref(Cpu6502_State &cs, Addr loc) {
    return {
            [&cs, loc]() { return cs.get_byte(loc); },
            [&cs, loc](Val x) { return cs.set_byte(loc, x); }
    };
}

void create_acc_suite(array<InstructionPtr, 256> res, int base_addr,
                      const function<void(Cpu6502_State &, AddrOrVal arg)> &func, bool sta) {
    if (!sta)
        res[base_addr + 0x09] = make_shared<FunctionInstruction>([&func](Cpu6502_State &cs) {
            Val imm = cs.get_instr_byte();
            func(cs, AddrOrVal::create_val(imm));
            return 2;
        });
    res[base_addr + 0x0D] = make_shared<FunctionInstruction>([&func, sta](Cpu6502_State &cs) {
        Addr addr = get_2b_addr(cs);
        func(cs, AddrOrVal::create(sta, addr, cs.get_byte(addr)));
        return 4;
    });
    res[base_addr + 0x1D] = make_shared<FunctionInstruction>([&func, sta](Cpu6502_State &cs) {
        auto [val, p] = x_indexed(cs, get_2b_addr(cs), sta);
        func(cs, val);
        if (sta) p = true;
        return 4 + p;
    });
    res[base_addr + 0x19] = make_shared<FunctionInstruction>([&func, sta](Cpu6502_State &cs) {
        auto [val, p] = y_indexed(cs, get_2b_addr(cs), sta);
        func(cs, val);
        if (sta) p = true;
        return 4 + p;
    });
    res[base_addr + 0x05] = make_shared<FunctionInstruction>([&func, sta](Cpu6502_State &cs) {
        auto addr = ZeroPageAddr(cs.get_instr_byte());
        func(cs, AddrOrVal::create(sta, Addr(addr.addr), cs.get_byte(addr)));
        return 3;
    });
    res[base_addr + 0x15] = make_shared<FunctionInstruction>([&func, sta](Cpu6502_State &cs) {
        auto [val, p] = x_indexed_zero_page(cs, ZeroPageAddr(cs.get_instr_byte()), sta);
        func(cs, val);
        return 4;
    });
    res[base_addr + 0x01] = make_shared<FunctionInstruction>([&func, sta](Cpu6502_State &cs) {
        func(cs, x_indexed_zero_page_indirect(cs, sta));
        return 6;
    });
    res[0x11] = make_shared<FunctionInstruction>([&func, sta](Cpu6502_State &cs) {
        auto [val, p] = zero_page_indirect_y_indexed(cs, sta);
        func(cs, val);
        if (sta) p = true;
        return 5 + p;
    });
}

void create_shift_suite(array<InstructionPtr, 256> res, int base_addr,
                        const function<void(Cpu6502_State &cs, ValReference ref)> &func, bool include_acc) {
    if (include_acc)
        res[base_addr + 0x0A] = make_shared<FunctionInstruction>([&func](Cpu6502_State &cs) {
            func(cs, acc_ref(cs));
            return 2;
        });
    res[base_addr + 0x0E] = make_shared<FunctionInstruction>([&func](Cpu6502_State &cs) {
        func(cs, mem_ref(cs, get_2b_addr(cs)));
        return 6;
    });
    res[base_addr + 0x1E] = make_shared<FunctionInstruction>([&func](Cpu6502_State &cs) {
        auto [addr, p] = x_indexed(cs, get_2b_addr(cs), true);
        func(cs, mem_ref(cs, addr.getAddr()));
        return 7;
    });
    res[base_addr + 0x06] = make_shared<FunctionInstruction>([&func](Cpu6502_State &cs) {
        auto addr = ZeroPageAddr(cs.get_instr_byte());
        func(cs, mem_ref(cs, Addr(addr.addr)));
        return 5;
    });
    res[base_addr + 0x16] = make_shared<FunctionInstruction>([&func](Cpu6502_State &cs) {
        auto [addr, p] = x_indexed_zero_page(cs, ZeroPageAddr(cs.get_instr_byte()), true);
        func(cs, mem_ref(cs, addr.getAddr()));
        return 6;
    });
}

//Fetch is assumed to run before this happens automatically
array<InstructionPtr, 256> instruction_ref() {
    array<InstructionPtr, 256> res{};

    create_acc_suite(res, 0x00, instr_ora, false);
    create_acc_suite(res, 0x20, instr_adc, false);
    create_acc_suite(res, 0x40, instr_eor, false);
    create_acc_suite(res, 0x60, instr_and, false);
    create_acc_suite(res, 0x80, instr_sta, true);
    create_acc_suite(res, 0xA0, instr_lda, false);
    create_acc_suite(res, 0xC0, instr_cmp, false);
    create_acc_suite(res, 0xE0, instr_sbc, false);

    create_shift_suite(res, 0x00, instr_asl, true);
    create_shift_suite(res, 0x00, instr_lsr, true);
    create_shift_suite(res, 0x00, instr_rol, true);
    create_shift_suite(res, 0x00, instr_ror, true);
    create_shift_suite(res, 0x00, instr_dec, false);
    create_shift_suite(res, 0x00, instr_inc, false);

    res[0xA2] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        instr_ldx(cs, cs.get_instr_byte());
        return 2;
    });
    res[0xAE] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        instr_ldx(cs, cs.get_byte(get_2b_addr(cs)));
        return 4;
    });
    res[0xBE] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        auto [val, p] = y_indexed(cs, get_2b_addr(cs), false);
        instr_ldx(cs, val.getVal());
        return 4 + p;
    });
    res[0xA6] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        instr_ldx(cs, cs.get_instr_byte());
        return 3;
    });
    res[0xB6] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        auto [val, p] = y_indexed_zero_page(cs, ZeroPageAddr(cs.get_instr_byte()), false);
        instr_ldx(cs, val.getVal());
        return 4;
    });

    res[0xA0] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        instr_ldy(cs, cs.get_instr_byte());
        return 2;
    });
    res[0xAC] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        instr_ldy(cs, cs.get_byte(get_2b_addr(cs)));
        return 4;
    });
    res[0xBC] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        auto [val, p] = y_indexed(cs, get_2b_addr(cs), false);
        instr_ldy(cs, val.getVal());
        return 4 + p;
    });
    res[0xA4] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        instr_ldy(cs, cs.get_instr_byte());
        return 3;
    });
    res[0xB4] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        auto [val, p] = y_indexed_zero_page(cs, ZeroPageAddr(cs.get_instr_byte()), false);
        instr_ldy(cs, val.getVal());
        return 4;
    });

    res[0x8E] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        instr_stx(cs, get_2b_addr(cs));
        return 4;
    });

    res[0x86] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        instr_stx(cs, Addr(cs.get_instr_byte().val));
        return 3;
    });

    res[0x96] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        auto aov = y_indexed_zero_page(cs, ZeroPageAddr(cs.get_instr_byte()), true);
        instr_stx(cs, aov.first.getAddr());
        return 4;
    });

    res[0x8C] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        instr_sty(cs, get_2b_addr(cs));
        return 4;
    });

    res[0x84] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        instr_sty(cs, Addr(cs.get_instr_byte().val));
        return 3;
    });

    res[0x94] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        auto aov = y_indexed_zero_page(cs, ZeroPageAddr(cs.get_instr_byte()), true);
        instr_sty(cs, aov.first.getAddr());
        return 4;
    });

    res[0xAA] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().setX(cs.reg().getA());
        cs.reg().set_flag(FlagPositions::NEG, cs.reg().getA().val & 0x80);
        cs.reg().set_flag(FlagPositions::ZERO, cs.reg().getA().val == 0);
        return 2;
    });

    res[0xA8] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().setY(cs.reg().getA());
        cs.reg().set_flag(FlagPositions::NEG, cs.reg().getA().val & 0x80);
        cs.reg().set_flag(FlagPositions::ZERO, cs.reg().getA().val == 0);
        return 2;
    });

    res[0xBA] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().setX(cs.reg().getS());
        cs.reg().set_flag(FlagPositions::NEG, cs.reg().getS().val & 0x80);
        cs.reg().set_flag(FlagPositions::ZERO, cs.reg().getS().val == 0);
        return 2;
    });

    res[0x8A] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().setA(cs.reg().getX());
        cs.reg().set_flag(FlagPositions::NEG, cs.reg().getX().val & 0x80);
        cs.reg().set_flag(FlagPositions::ZERO, cs.reg().getX().val == 0);
        return 2;
    });

    res[0x9A] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().setS(cs.reg().getX());
        cs.reg().set_flag(FlagPositions::NEG, cs.reg().getX().val & 0x80);
        cs.reg().set_flag(FlagPositions::ZERO, cs.reg().getX().val == 0);
        return 2;
    });

    res[0x98] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().setA(cs.reg().getY());
        cs.reg().set_flag(FlagPositions::NEG, cs.reg().getY().val & 0x80);
        cs.reg().set_flag(FlagPositions::ZERO, cs.reg().getY().val == 0);
        return 2;
    });

    res[0x48] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.set_byte(Addr(cs.reg().getS().val), cs.reg().getA());
        cs.reg().setS(cs.reg().getS() - Val(1));
        return 3;
    });

    res[0x08] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.set_byte(Addr(cs.reg().getS().val), cs.reg().getP());
        cs.reg().setS(cs.reg().getS() - Val(1));
        return 3;
    });

    res[0x68] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().setS(cs.reg().getS() + Val(1));
        cs.reg().setA(cs.get_byte(Addr(cs.reg().getS().val)));
        return 4;
    });

    res[0x28] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().setS(cs.reg().getS() + Val(1));
        cs.reg().setP(cs.get_byte(Addr(cs.reg().getS().val)));
        return 4;
    });
    res[0x2C] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        Addr addy = get_2b_addr(cs);
        instr_bit(cs, cs.get_byte(addy));
        return 4;
    });
    res[0x24] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        Addr addy = Addr(cs.get_instr_byte().val);
        instr_bit(cs, cs.get_byte(addy));
        return 3;
    });
    res[0xE0] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        Val valve = cs.get_instr_byte();
        instr_cpx(cs, valve);
        return 2;
    });
    res[0xEC] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        Val valve = cs.get_byte(get_2b_addr(cs));
        instr_cpx(cs, valve);
        return 4;
    });
    res[0xE4] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        Val valve = cs.get_byte(Addr(cs.get_instr_byte().val));
        instr_cpx(cs, valve);
        return 3;
    });

    res[0xC0] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        Val valve = cs.get_instr_byte();
        instr_cpy(cs, valve);
        return 2;
    });
    res[0xCC] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        Val valve = cs.get_byte(get_2b_addr(cs));
        instr_cpy(cs, valve);
        return 4;
    });
    res[0xC4] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        Val valve = cs.get_byte(Addr(cs.get_instr_byte().val));
        instr_cpy(cs, valve);
        return 3;
    });
    res[0xCA] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().setX(cs.reg().getX() - Val(1));
        cs.reg().set_flag(FlagPositions::NEG, cs.reg().getX().val & 0x80);
        cs.reg().set_flag(FlagPositions::ZERO, cs.reg().getX().val == 0);
        return 2;
    });

    res[0x88] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().setY(cs.reg().getY() - Val(1));
        cs.reg().set_flag(FlagPositions::NEG, cs.reg().getY().val & 0x80);
        cs.reg().set_flag(FlagPositions::ZERO, cs.reg().getY().val == 0);
        return 2;
    });

    res[0xE8] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().setX(cs.reg().getX() + Val(1));
        cs.reg().set_flag(FlagPositions::NEG, cs.reg().getX().val & 0x80);
        cs.reg().set_flag(FlagPositions::ZERO, cs.reg().getX().val == 0);
        return 2;
    });

    res[0xC8] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().setY(cs.reg().getY() + Val(1));
        cs.reg().set_flag(FlagPositions::NEG, cs.reg().getY().val & 0x80);
        cs.reg().set_flag(FlagPositions::ZERO, cs.reg().getY().val == 0);
        return 2;
    });

    res[0x4C] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        Addr new_pc = get_2b_addr(cs);
        cs.reg().setPC(new_pc);
        return 3;
    });

    res[0x6C] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        Addr new_pc = indirect_addr_jump(cs, get_2b_addr(cs));
        cs.reg().setPC(new_pc);
        return 5;
    });

    res[0x20] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        Addr new_pc = get_2b_addr(cs);
        cs.push_stack(Val(static_cast<uint8_t>(cs.reg().getPC().addr >> 8)));
        cs.push_stack(Val(static_cast<uint8_t>(cs.reg().getPC().addr & 0xFF)));
        cs.reg().setPC(new_pc);
        return 6;
    });

    res[0x60] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        Val low = cs.pull_stack();
        Val high = cs.pull_stack();
        Addr PC = Addr((static_cast<uint16_t>(high.val) << 8) | static_cast<uint16_t>(low.val));
        cs.reg().setPC(PC);
        return 6;
    });

    res[0x90] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        return branch(cs, [](auto &cs) { return !cs.reg().get_flag(FlagPositions::CARRY); });
    });
    res[0xB0] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        return branch(cs, [](auto &cs) { return cs.reg().get_flag(FlagPositions::CARRY); });
    });
    res[0xF0] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        return branch(cs, [](auto &cs) { return cs.reg().get_flag(FlagPositions::ZERO); });
    });
    res[0x30] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        return branch(cs, [](auto &cs) { return cs.reg().get_flag(FlagPositions::NEG); });
    });
    res[0xD0] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        return branch(cs, [](auto &cs) { return !cs.reg().get_flag(FlagPositions::ZERO); });
    });
    res[0x10] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        return branch(cs, [](auto &cs) { return !cs.reg().get_flag(FlagPositions::NEG); });
    });
    res[0x50] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        return branch(cs, [](auto &cs) { return !cs.reg().get_flag(FlagPositions::OVF); });
    });
    res[0x70] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        return branch(cs, [](auto &cs) { return cs.reg().get_flag(FlagPositions::OVF); });
    });
    res[0x18] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().set_flag(FlagPositions::CARRY, false);
        return 2;
    });
    res[0xD8] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().set_flag(FlagPositions::DECIMAL, false);
        return 2;
    });
    res[0x58] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().set_flag(FlagPositions::INTERRUPT_DISABLE, false);
        return 2;
    });
    res[0xB8] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().set_flag(FlagPositions::OVF, false);
        return 2;
    });
    res[0x38] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().set_flag(FlagPositions::CARRY, true);
        return 2;
    });
    res[0xF8] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().set_flag(FlagPositions::DECIMAL, true);
        return 2;
    });
    res[0x78] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().set_flag(FlagPositions::INTERRUPT_DISABLE, true);
        return 2;
    });
    res[0xEA] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        //Fabled NOP
        return 2;
    });
    //Interrupts: Last but not least
    res[0x00] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        Addr return_addr = Addr(cs.reg().getPC().addr + 1);

        cs.push_stack(Val(static_cast<uint8_t>(return_addr.addr >> 8)));
        cs.push_stack(Val(static_cast<uint8_t>(return_addr.addr & 0xFF)));

        cs.reg().set_flag(FlagPositions::B, true);
        cs.push_stack(Val(cs.reg().getP()));
        cs.reg().set_flag(FlagPositions::B, false);

        cs.reg().set_flag(FlagPositions::INTERRUPT_DISABLE, true);

        uint8_t low = cs.get_byte(Addr(0xFFFE)).val;
        uint8_t high = cs.get_byte(Addr(0xFFFF)).val;
        Addr new_pc = Addr((static_cast<uint16_t>(high) << 8) | low);
        cs.reg().setPC(new_pc);

        return 7;
    });
    res[0x40] = make_shared<FunctionInstruction>([](Cpu6502_State &cs) {
        cs.reg().setP(cs.pull_stack());
        cs.reg().set_flag(FlagPositions::B, false);
        uint8_t low = cs.pull_stack().val;
        uint8_t high = cs.pull_stack().val;
        Addr new_pc = Addr((static_cast<uint16_t>(high) << 8) | low);

        // Set the Program Counter to the new address
        cs.reg().setPC(new_pc);
        return 6;
    });
    return res;
}

const array<InstructionPtr, 256> instructions = instruction_ref();
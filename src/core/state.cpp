#pragma once

#include "array"
#include "basics.h"
#include <iostream>
#include "state.h"

using namespace std;

bool Reg::get_flag(FlagPositions flag) const {
    return (P.val >> static_cast<int>(flag)) & 1;
}

void Reg::set_flag(FlagPositions flag, bool value) {
    if (value)
        P.val |= 1 << static_cast<int>(flag);
    else
        P.val &= ~(1 << static_cast<int>(flag));
}

Val Reg::getA() const {
    return A;
}

void Reg::setA(Val a) {
    A = a;
}

Val Reg::getX() const {
    return X;
}

void Reg::setX(Val x) {
    X = x;
}

Val Reg::getY() const {
    return Y;
}

void Reg::setY(Val y) {
    Y = y;
}

Addr Reg::getPC() const {
    return PC;
}

void Reg::setPC(Addr pc) {
    PC = pc;
}

void Reg::incrPC() {
    PC.addr++;
}

Val Reg::getS() const {
    return S;
}

void Reg::setS(Val s) {
    S = s;
}

Val Reg::getP() const {
    return P;
}

void Reg::setP(Val p) {
    P = p;
}


void Memory::write_byte(uint16_t address, uint8_t value) {
//    if (address < 0x2000) {
//        ram[address % RAM_SIZE] = value;
//        written[address % RAM_SIZE] = true;
//    } else if (address < 0x4000) {
//        ppu_registers[(address - 0x2000) % PPU_REGISTERS_SIZE] = value;
//    } else if (address < 0x4020) {
//        apu_io_registers[address - 0x4000] = value;
//    } else if (address >= 0xFFFA && address <= 0xFFFF) {
//        interrupt_vec[address - 0xFFFA] = value;
//    } else if (address >= 0x8000) {
//        prg_rom[address - 0x8000] = value;
//    } else {
    misc_mem[address] = value;
    //}
}

uint8_t Memory::read_byte(uint16_t address) {
//    if (address < 0x2000) {
//        if (!written[address % RAM_SIZE])
//            throw invalid_argument("Ram not initialized at address " + to_string(address));
//        return ram[address % RAM_SIZE];
//    } else if (address < 0x4000) {
//        return ppu_registers[(address - 0x2000) % PPU_REGISTERS_SIZE];
//    } else if (address < 0x4020) {
//        return apu_io_registers[address - 0x4000];
//    } else if (address >= 0xFFFA && address <= 0xFFFF) {
//        return interrupt_vec[address - 0xFFFA];
//    } else if (address >= 0x8000) {
//        return prg_rom[address - 0x8000];
//    } else {
    return misc_mem[address];
    //}
}

void Memory::loadCartridge(const std::vector<uint8_t> &romData) {
    // Assuming PRG-ROM data starts at the beginning of romData
    std::copy(romData.begin(), romData.begin() + PRG_ROM_SIZE, prg_rom.begin());
}

void Memory::power() {
    for (int i = 0; i < RAM_SIZE; i++)
        ram[i] = 0;
    std::fill(interrupt_vec.begin(), interrupt_vec.end(), 0xFF);
}


void Cpu6502_State::set_byte(Addr loc, Val data) {
    m.write_byte(loc.addr, data.val);
}

[[nodiscard]] Val Cpu6502_State::get_byte(Addr loc) {
    return Val(m.read_byte(loc.addr));
}

[[nodiscard]] Val Cpu6502_State::get_byte(ZeroPageAddr loc) {
    return Val(m.read_byte(loc.addr));
}

Reg &Cpu6502_State::reg() {
    return r;
}

Val Cpu6502_State::get_instr_byte() {
    Val ret = get_byte(r.getPC());
    r.incrPC();
    return ret;
}

Val Cpu6502_State::add(Val left, Val right) {
    Val carry = Val(r.get_flag(FlagPositions::CARRY) ? 1 : 0);
    Val res = left + right + carry;
    uint16_t ovf = (uint16_t) left.val + (uint16_t) right.val + carry.val;
    r.set_flag(FlagPositions::CARRY, ovf > 0xFF);
    r.set_flag(FlagPositions::ZERO, res.val == 0);
    r.set_flag(FlagPositions::NEG, res.val & 0x80);
    r.set_flag(FlagPositions::OVF, ((left ^ res) & (right ^ res) & Val(0x80)) != Val(0));
    return res;
}

void Cpu6502_State::push_stack(Val val) {
    uint16_t stack_addr = 0x0100 + (r.getS().val % 0x100);
    m.write_byte(stack_addr, val.val);
    r.setS(r.getS() - Val(1));
}

Val Cpu6502_State::pull_stack() {
    r.setS(r.getS() + Val(1));
    uint16_t stack_addr = 0x0100 + (r.getS().val % 0x100);
    return Val(m.read_byte(stack_addr));
}

Cpu6502_State::Cpu6502_State() {
    m = {};
    r = Reg();
}

Memory &Cpu6502_State::mem() {
    return m;
}
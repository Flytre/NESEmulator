#include "array"
#include "basics.h"
#include <iostream>

#ifndef NESEMULATOR_STATE_H
#define NESEMULATOR_STATE_H

constexpr uint16_t RAM_SIZE = 0x0800; // 2KB internal RAM
constexpr uint16_t PPU_REGISTERS_SIZE = 8; // PPU registers
constexpr uint16_t APU_IO_REGISTERS_SIZE = 0x18; // APU and I/O registers
constexpr uint16_t PRG_ROM_SIZE = 0x8000;

enum class FlagPositions {
    CARRY = 0,
    ZERO = 1,
    INTERRUPT_DISABLE = 2,
    DECIMAL = 3,
    B = 4,
    UNUSED = 5,
    OVF = 6,
    NEG = 7
};

class Reg {
public:
    Reg() = default;

    [[nodiscard]] bool get_flag(FlagPositions flag) const;

    void set_flag(FlagPositions flag, bool value);

    [[nodiscard]] Val getA() const;

    void setA(Val a);

    [[nodiscard]] Val getX() const;

    void setX(Val x);

    [[nodiscard]] Val getY() const;

    void setY(Val y);

    [[nodiscard]] Addr getPC() const;

    void setPC(Addr pc);

    void incrPC();

    [[nodiscard]] Val getS() const;

    void setS(Val s);

    [[nodiscard]] Val getP() const;

    void setP(Val p);
private:
    Val A;
    Val X;
    Val Y;
    Addr PC;
    Val S;
    Val P;
};

class Memory {
public:
    Memory() = default;

    void write_byte(uint16_t address, uint8_t value);

    uint8_t read_byte(uint16_t address);

    void loadCartridge(const std::vector<uint8_t> &romData);

    void power();
private:
    std::array<uint8_t, RAM_SIZE> ram = {};
    std::array<bool, RAM_SIZE> written = {};
    std::array<uint8_t, PPU_REGISTERS_SIZE> ppu_registers = {};
    std::array<uint8_t, APU_IO_REGISTERS_SIZE> apu_io_registers = {};
    std::array<uint8_t, PRG_ROM_SIZE> prg_rom = {};
    std::array<uint8_t, 6> interrupt_vec = {};
    std::array<uint8_t, 0x10000> misc_mem = {};

};


class Cpu6502_State {
public:
    void set_byte(Addr loc, Val data);

    [[nodiscard]] Val get_byte(Addr loc);

    [[nodiscard]] Val get_byte(ZeroPageAddr loc);

    Reg &reg();

    Val get_instr_byte();

    Val add(Val left, Val right);

    void push_stack(Val val);

    Val pull_stack();

    Cpu6502_State();

    Memory& mem();
private:
    Memory m;
    Reg r;
};

#endif //NESEMULATOR_STATE_H

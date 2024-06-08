//
// Created by Aaron Rahman on 6/7/24.
//
#include "array"
#include "basics.cpp"
#include <iostream>

using namespace std;

//TODO: DECIMAL MODE
enum class FlagPositions {
    CARRY = 0,
    ZERO = 1,
    INTERRUPT_DISABLE = 2,
    DECIMAL = 3,
    B = 4,
    OVF = 6,
    NEG = 7
};

constexpr uint16_t RAM_SIZE = 0x0800; // 2KB internal RAM
constexpr uint16_t PPU_REGISTERS_SIZE = 8; // PPU registers
constexpr uint16_t APU_IO_REGISTERS_SIZE = 0x18; // APU and I/O registers
constexpr uint16_t PRG_ROM_SIZE = 0x8000;

class Reg {
private:
    Val A;
    Val X;
    Val Y;
    Addr PC;
    Val S; //technically 8 bits
    Val P;
public:
    Reg() {}

    [[nodiscard]] bool get_flag(FlagPositions flag) const {
        return (P.val >> static_cast<int>(flag)) & 1;
    }

    void set_flag(FlagPositions flag, bool value) {
        if (value)
            P.val |= 1 << static_cast<int>(flag);
        else
            P.val &= ~(1 << static_cast<int>(flag));
    }

    [[nodiscard]] Val getA() const {
        return A;
    }

    void setA(Val a) {
        A = a;
    }

    [[nodiscard]] Val getX() const {
        return X;
    }

    void setX(Val x) {
        X = x;
    }

    [[nodiscard]] Val getY() const {
        return Y;
    }

    void setY(Val y) {
        Y = y;
    }

    [[nodiscard]] Addr getPC() const {
        return PC;
    }

    void setPC(Addr pc) {
        PC = pc;
    }

    void incrPC() {
        PC.addr++;
    }


    [[nodiscard]] Val getS() const {
        return S;
    }

    void setS(Val s) {
        S = s;
    }

    [[nodiscard]] Val getP() const {
        return P;
    }

    void setP(Val p) {
        P = p;
    }
};

class Memory {
private:
    array<uint8_t, RAM_SIZE> ram = {};
    array<bool, RAM_SIZE> written = {};
    array<uint8_t, PPU_REGISTERS_SIZE> ppu_registers = {};
    array<uint8_t, APU_IO_REGISTERS_SIZE> apu_io_registers = {};
    array<uint8_t, PRG_ROM_SIZE> prg_rom = {};
public:
    Memory() {}

    void write_byte(uint16_t address, uint8_t value) {
        if (address < 0x2000) {
            ram[address % RAM_SIZE] = value;
            written[address % RAM_SIZE] = true;
        } else if (address < 0x4000) {
            ppu_registers[(address - 0x2000) % PPU_REGISTERS_SIZE] = value;
        } else if (address < 0x4020) {
            apu_io_registers[address - 0x4000] = value;
        } else {
            std::cerr << "Unhandled memory write at address: " << std::hex << address << " value: " << std::hex
                      << (int) value << "\n";
        }
    }

    uint8_t read_byte(uint16_t address) {
        if (address < 0x2000) {
            if (!written[address % RAM_SIZE])
                throw invalid_argument("Ram not initialized at address " + to_string(address));
            return ram[address % RAM_SIZE];
        } else if (address < 0x4000) {
            return ppu_registers[(address - 0x2000) % PPU_REGISTERS_SIZE];
        } else if (address < 0x4020) {
            return apu_io_registers[address - 0x4000];
        } else if (address >= 0x8000) {
            return prg_rom[address - 0x8000];
        } else {
            std::cerr << "Unhandled memory read at address: " << std::hex << address << "\n";
            return 0; // Placeholder for unhandled memory regions
        }
    }

    void loadCartridge(const std::vector<uint8_t> &romData) {
        // Assuming PRG-ROM data starts at the beginning of romData
        std::copy(romData.begin(), romData.begin() + PRG_ROM_SIZE, prg_rom.begin());
    }

    void power() {
        for (int i = 0; i < RAM_SIZE; i++)
            ram[i] = 0;
    }

};

class Cpu6502_State {
public:
    Memory mem;
    Reg r;

    void set_byte(Addr loc, Val data) {
        mem.write_byte(loc.addr, data.val);
    }

    [[nodiscard]] Val get_byte(Addr loc) {
        return Val(mem.read_byte(loc.addr));
    }

    [[nodiscard]] Val get_byte(ZeroPageAddr loc) {
        return Val(mem.read_byte(loc.addr));
    }

    Reg &reg() {
        return r;
    }

    Val get_instr_byte() {
        Val ret = get_byte(r.getPC());
        r.incrPC();
        return ret;
    }

    Val add(Val left, Val right) {
        if (r.get_flag(FlagPositions::DECIMAL))
            throw std::invalid_argument("Operation Not Supported");
        Val carry = Val(r.get_flag(FlagPositions::CARRY) ? 1 : 0);
        Val res = left + right + carry;
        uint16_t ovf = (uint16_t) left.val + (uint16_t) right.val + carry.val;
        r.set_flag(FlagPositions::CARRY, ovf > 0xFF);
        r.set_flag(FlagPositions::ZERO, res.val == 0);
        r.set_flag(FlagPositions::NEG, res.val & 0x80);
        r.set_flag(FlagPositions::OVF, ((left ^ res) & (right ^ res) & Val(0x80)) != Val(0));
        return res;
    }

    void push_stack(Val val) {
        mem.write_byte(r.getS().val, val.val);
        r.setS(r.getS() - Val(1));
    }

    Val pull_stack() {
        r.setS(r.getS() + Val(1));
        return Val(mem.read_byte(r.getS().val));
    }

    Cpu6502_State() {
        mem = {};
        r = Reg();
    }

    void power() {
        r.setA(Val(0));
        r.setX(Val(0));
        r.setY(Val(0));
        r.setPC(Addr(0xFFF3));
        r.setS(Val(0xFD));
        r.setP(Val(4));
        mem.power();
    }
};
#pragma once
#include "state.h"
#include "instructions.h"
#include <vector>
#include <iostream>

using namespace std;


class Cpu6502 {
private:
    uint8_t _instr_ = 0x00;
    int _cycle_ct_ = 0x00;
    Cpu6502_State state;
public:
    void power() {
        state.reg().setA(Val(0));
        state.reg().setX(Val(0));
        state.reg().setY(Val(0));
        state.reg().setPC(Addr(0xFFF3));
        state.reg().setS(Val(0xFD));
        state.reg().setP(Val(4));
        state.mem().power();
    }

    void posedge_clock() {
        if (_cycle_ct_ == 0) {
            _instr_ = state.get_byte(state.reg().getPC()).val;
            state.reg().incrPC();
            _cycle_ct_ = instructions[_instr_]->act(state);
        }
        _cycle_ct_--;
    }

    void load_rom(istream &stream) {
        vector<char> rom_data((istreambuf_iterator<char>(stream)), istreambuf_iterator<char>());

        if (rom_data.size() < 16)
            throw invalid_argument("ROM doesn't contain a header.");
        int prg_rom_size = 16384 * rom_data[4];
        int chr_rom_size = 8192 * rom_data[5];

        const int prg_rom_start = 16;
        const int prg_rom_end = prg_rom_start + prg_rom_size;

        size_t expected_size = 16 + prg_rom_size + (chr_rom_size ? chr_rom_size : 0);
        if (rom_data.size() < expected_size) {
            throw std::runtime_error("ROM file size does not match header information");
        }

        if (prg_rom_size == 0x4000) {
            for (int i = 0; i < prg_rom_end - prg_rom_start; i++) {
                state.mem().write_byte(0x8000 + i, rom_data[prg_rom_start + i]);
                state.mem().write_byte(0xC000 + i, rom_data[prg_rom_start + i]);
            }
        } else if (prg_rom_size == 0x8000) {
            for (int i = 0; i < prg_rom_end - prg_rom_start; i++) {
                state.mem().write_byte(0x8000 + i, rom_data[prg_rom_start + i]);
            }
        } else {
            throw std::runtime_error("Unsupported PRG ROM size");
        }

        std::cout << "ROM loaded successfully" << std::endl;
    }
};
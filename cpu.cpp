#include "state.cpp"
#include "instructions.cpp"

using namespace std;


class Cpu6502 {
private:
    uint8_t _instr_ = 0x00;
    int _cycle_ct_ = 0x00;
    Cpu6502_State state;
public:
    void power() {
        state.power();
    }

    void posedge_clock() {
        if (_cycle_ct_ == 0) {
            _instr_ = state.get_byte(state.reg().getPC()).val;
            state.reg().incrPC();
            _cycle_ct_ = instructions[_instr_]->act(state);
        }
        _cycle_ct_--;
    }
};
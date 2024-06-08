#include "array"

using reg1b = unsigned char;
using reg2b = unsigned short;
using mem_entry = unsigned char;
using namespace std;


class Cpu6502 {
    class Reg {
    public:
        reg1b A;
        reg1b X;
        reg1b Y;
        reg2b PC;
        reg1b S;
        reg1b P;
    };

public:
    array<int, 1> mem;
    Reg reg;
};
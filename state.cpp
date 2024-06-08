//
// Created by Aaron Rahman on 6/7/24.
//
#include "array"
#include "basics.cpp"

using namespace std;
using cpu_mem = array<Val, 1 << 16>;

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

class Cpu6502_State {
    class Reg {
    private:
        Val A;
        Val X;
        Val Y;
        Addr PC;
        Val S; //technically 8 bits
        Val P;
    public:
        Reg() {
            P = Val(1 << 5);
            S = Val(0xff);
        }

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

public:
    cpu_mem mem;
    Reg r;

    void set_byte(Addr loc, Val data) {
        mem[loc.addr] = data;
    }

    [[nodiscard]] Val get_byte(Addr loc) {
        return mem[loc.addr];
    }

    [[nodiscard]] Val get_byte(ZeroPageAddr loc) {
        return mem[loc.addr];
    }

    Reg &reg() {
        return r;
    }

    Val get_instr_byte() {
        Val ret = mem[r.getPC().addr];
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
        mem[r.getS().val] = val;
        r.setS(r.getS() - Val(1));
    }

    Val pull_stack() {
        r.setS(r.getS() + Val(1));
        return mem[r.getS().val];
    }

    Cpu6502_State() {
        mem = cpu_mem();
        r = Reg();
    }
};
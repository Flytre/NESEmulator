//
// Created by Aaron Rahman on 6/7/24.
//
#include "array"

using namespace std;
using cpu_mem = array<uint8_t, 1 << 16>;

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
        uint8_t A;
        uint8_t X;
        uint8_t Y;
        uint16_t PC;
        uint8_t S;
        uint8_t P;
    public:
        Reg() {
            P = 1 << 5;
            S = 0xff;
        }

        [[nodiscard]] bool flag_set(FlagPositions flag) const {
            return (P >> static_cast<int>(flag)) & 1;
        }

        void set_flag(FlagPositions flag, bool value) {
            if (value)
                P |= 1 << static_cast<int>(flag);
            else
                P &= ~(1 << static_cast<int>(flag));
        }

        [[nodiscard]] uint8_t getA() const {
            return A;
        }

        void setA(uint8_t a) {
            A = a;
        }

        [[nodiscard]] uint8_t getX() const {
            return X;
        }

        void setX(uint8_t x) {
            X = x;
        }

        [[nodiscard]] uint8_t getY() const {
            return Y;
        }

        void setY(uint8_t y) {
            Y = y;
        }

        [[nodiscard]] uint16_t getPC() const {
            return PC;
        }

        void setPC(uint16_t pc) {
            PC = pc;
        }

        void incrPC() {
            PC++;
        }


        [[nodiscard]] uint8_t getS() const {
            return S;
        }

        void setS(uint8_t s) {
            S = s;
        }

        [[nodiscard]] uint8_t getP() const {
            return P;
        }

        void setP(uint8_t p) {
            P = p;
        }
    };

public:
    cpu_mem mem;
    Reg reg;

    const cpu_mem &getMem() {
        return mem;
    }

    Reg &getReg() {
        return reg;
    }

    Cpu6502_State() {
        mem = cpu_mem();
        reg = Reg();
    }
};
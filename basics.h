#ifndef NESEMULATOR_BASICS_H
#define NESEMULATOR_BASICS_H

#include <utility>
#include <functional>
#include <stdexcept>

struct Addr {
    uint16_t addr;

    Addr() : addr(0) {}

    explicit Addr(uint16_t addr) : addr(addr) {}

    // Overload the addition operator
    Addr operator+(const Addr &other) const {
        return Addr(this->addr + other.addr);
    }

    // Overload the subtraction operator
    Addr operator-(const Addr &other) const {
        return Addr(this->addr - other.addr);
    }

    // Overload the left shift operator
    Addr operator<<(int shift) const {
        return Addr(this->addr << shift);
    }

    // Overload the right shift operator
    Addr operator>>(int shift) const {
        return Addr(this->addr >> shift);
    }

    // Overload the bitwise OR operator
    Addr operator|(const Addr &other) const {
        return Addr(this->addr | other.addr);
    }
};

struct Val {
    uint8_t val;

    Val() : val(0) {}

    explicit Val(uint8_t val) : val(val) {}

    // Overload the addition operator
    Val operator+(const Val &other) const {
        return Val(this->val + other.val);
    }

    // Overload the subtraction operator
    Val operator-(const Val &other) const {
        return Val(this->val - other.val);
    }

    // Overload the equality operator
    bool operator==(const Val &other) const {
        return this->val == other.val;
    }

    // Overload the inequality operator
    bool operator!=(const Val &other) const {
        return !(*this == other);
    }

    // Overload the less than operator
    bool operator<(const Val &other) const {
        return this->val < other.val;
    }

    // Overload the less than or equal to operator
    bool operator<=(const Val &other) const {
        return this->val <= other.val;
    }

    // Overload the greater than operator
    bool operator>(const Val &other) const {
        return this->val > other.val;
    }

    // Overload the greater than or equal to operator
    bool operator>=(const Val &other) const {
        return this->val >= other.val;
    }

    // Overload the bitwise AND operator
    Val operator&(const Val &other) const {
        return Val(this->val & other.val);
    }

    // Overload the bitwise OR operator
    Val operator|(const Val &other) const {
        return Val(this->val | other.val);
    }

    // Overload the bitwise XOR operator
    Val operator^(const Val &other) const {
        return Val(this->val ^ other.val);
    }

    // Overload the bitwise NOT operator
    Val operator~() const {
        return Val(~this->val);
    }

    // Overload the left shift operator
    Val operator<<(int shift) const {
        return Val(this->val << shift);
    }

    // Overload the right shift operator
    Val operator>>(int shift) const {
        return Val(this->val >> shift);
    }

    // Overload the bitwise AND assignment operator
    Val &operator&=(const Val &other) {
        this->val &= other.val;
        return *this;
    }

    // Overload the bitwise OR assignment operator
    Val &operator|=(const Val &other) {
        this->val |= other.val;
        return *this;
    }

    // Overload the bitwise XOR assignment operator
    Val &operator^=(const Val &other) {
        this->val ^= other.val;
        return *this;
    }

    // Overload the left shift assignment operator
    Val &operator<<=(int shift) {
        this->val <<= shift;
        return *this;
    }

    // Overload the right shift assignment operator
    Val &operator>>=(int shift) {
        this->val >>= shift;
        return *this;
    }
};

struct ZeroPageAddr {
    uint8_t addr;

    ZeroPageAddr() : addr(0) {}

    explicit ZeroPageAddr(Val addr) : addr(addr.val) {}

    // Overload the addition operator
    ZeroPageAddr operator+(const ZeroPageAddr &other) const {
        return ZeroPageAddr(Val(this->addr + other.addr));
    }

    // Overload the subtraction operator
    ZeroPageAddr operator-(const ZeroPageAddr &other) const {
        return ZeroPageAddr(Val(this->addr - other.addr));
    }
};

class ValReference {
private:
    std::function<Val()> getter;
    std::function<void(Val)> setter;

public:
    ValReference(std::function<Val()> getter, std::function<void(Val)> setter)
            : getter(std::move(getter)), setter(std::move(setter)) {}

    [[nodiscard]] Val get() const {
        return getter();
    }

    void set(Val value) {
        setter(value);
    }
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
        return {true, addr, Val(0)};
    };

    static AddrOrVal create_val(Val val) {
        return {true, Addr(0), val};
    };

    static AddrOrVal create(bool is_address, Addr addr, Val val) {
        return is_address ? create_addr(addr) : create_val(val);
    };

    Addr getAddr() {
        if (!is_addr)
            throw std::invalid_argument("Not an address");
        return addr;
    };

    Val getVal() {
        if (is_addr)
            throw std::invalid_argument("Not a value");
        return val;
    };
};

#endif //NESEMULATOR_BASICS_H

#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <iomanip>
#include <string>

int roundType = 0;

class FixedPoint {
private:
    int32_t raw;
    int A;
    int B;
    static int64_t round(int64_t num, int64_t den, int mode) {
        int64_t quotient = num / den;
        int64_t remainder = num - quotient * den;
        if (remainder == 0 || mode == 0) {
            return quotient;
        }
        int signFactor = ((num >= 0 && den > 0) || (num < 0 && den < 0)) ? 1 : -1;
        int64_t absRem = (remainder < 0) ? -remainder : remainder;
        int64_t absDen = (den < 0) ? -den : den;
        int64_t doubledRem = absRem << 1;
        int adjustment = 0;
        if (mode == 1) {
            if (doubledRem > absDen) {
                adjustment = signFactor;
            } else if (doubledRem == absDen) {
                adjustment = (quotient & 1LL) ? signFactor : 0;
            }
        } else if (mode == 2) {
            adjustment = (signFactor > 0) ? 1 : 0;
        } else if (mode == 3) {
            adjustment = (signFactor < 0) ? -1 : 0;
        }
        return quotient + adjustment;
    }
    
    static int64_t clamp(int64_t val, int A, int B) {
        int64_t mod = static_cast<int64_t>(1) << A + B;
        int64_t rem = val % mod;
        if (rem < 0) {
            rem += mod;
        }
        int64_t signBit = static_cast<int64_t>(1) << (A + B - 1);
        if (rem & signBit) {
            return rem - mod;
        }
        return rem;
    }
    
public:
    FixedPoint(uint32_t hexVal, int A, int B) : A(A), B(B) {
        uint32_t mask;
        if(A + B < 32)
            mask = (static_cast<uint32_t>(1) << A + B) - 1;
        else
            mask = 0xFFFFFFFFu;
        uint32_t valMasked = hexVal & mask;
        if (A + B < 32) {
            uint32_t signBit = static_cast<uint32_t>(1) << (A + B - 1);
            if (valMasked & signBit) {
                int32_t tmp = static_cast<int32_t>(valMasked) - (static_cast<int32_t>(1) << A + B);
                raw = tmp;
            } else {
                raw = static_cast<int32_t>(valMasked);
            }
        } else {
            raw = static_cast<int32_t>(valMasked);
        }
    }
    bool isZero() const {
        return raw == 0;
    }
    FixedPoint operator+(const FixedPoint &other) const {
        int64_t sum = static_cast<int64_t>(raw) + other.raw;
        sum = clamp(sum, A, B);
        return FixedPoint(static_cast<int32_t>(sum), A, B);
    }
    
    FixedPoint operator-(const FixedPoint &other) const {
        int64_t diff = static_cast<int64_t>(raw) - other.raw;
        diff = clamp(diff, A, B);
        return FixedPoint(static_cast<int32_t>(diff), A, B);
    }
    
    FixedPoint operator*(const FixedPoint &other) const {
        int64_t mul = static_cast<int64_t>(raw) * other.raw;
        int64_t res = round(mul, static_cast<int64_t>(1) << B, roundType);
        res = clamp(res, A, B);
        return FixedPoint(static_cast<int32_t>(res), A, B);
    }
    
    FixedPoint operator/(const FixedPoint &other) const {
        if (other.raw == 0) {
            return FixedPoint(0, A, B);
        }
        int64_t dividend = static_cast<int64_t>(raw) << B;
        int64_t divisor = static_cast<int64_t>(other.raw);
        int64_t res = round(dividend, divisor, roundType);
        res = clamp(res, A, B);
        return FixedPoint(static_cast<int32_t>(res), A, B);
    }
    
    void print() const {
        int64_t mod = static_cast<int64_t>(1) << (A + B);
        int64_t value = raw % mod;
        if (value < 0) {
            value += mod;
        }
        if (value & (static_cast<int64_t>(1) << ((A + B) - 1))) {
            value -= mod;
        }
        int64_t scaled = value * 1000;
        int64_t scale = static_cast<int64_t>(1) << B;
        int64_t rounded = round(scaled, scale, roundType);
        bool negative = (rounded < 0);
        if (negative) {
            rounded = -rounded;
        }
        int64_t intPart = rounded / 1000;
        int64_t fracPart = rounded % 1000;
        if (negative) {
            std::cout << "-";
        }
        std::cout << intPart << "."
                  << std::setw(3) << std::setfill('0') << fracPart
                  << "\n";
    }
};
int main(int argc, char* argv[])
{

    int i = 1;
    std::string format = argv[i++];
    auto dotPos = format.find('.');
    int A = std::stoi(format.substr(0, dotPos));
    int B = std::stoi(format.substr(dotPos + 1));
    roundType = std::atoi(argv[i++]);
    bool isOperation = (argc == 6);
    char op = 0;
    uint32_t val1 = 0, val2 = 0;
    if (isOperation) {
        op   = argv[i++][0];
        val1 = std::strtoul(argv[i++], nullptr, 16);
        val2 = std::strtoul(argv[i++], nullptr, 16);
    } else {
        val1 = std::strtoul(argv[i++], nullptr, 16);
    }
    FixedPoint num1(val1, A, B);
    if (isOperation) {
        FixedPoint num2(val2, A, B);
        FixedPoint result(0, A, B);
        if (op == '+') {
            result = num1 + num2;
        } else if (op == '-') {
            result = num1 - num2;
        } else if (op == '*') {
            result = num1 * num2;
        } else if (op == '/') {
            if (num2.isZero()) {
                std::cout << "div_by_zero\n";
                return 0;
            }
            result = num1 / num2;
        }
        result.print();
    } else {
        num1.print();
    }
    return 0;
}

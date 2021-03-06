#include <iostream>
#include "../src/bigint.hpp"
// test &, |, ^, <<, >>, ~
void do_test(calc::BigInt<>& a, calc::BigInt<>& b) {
    std::cout << std::hex << std::showbase;
    std::cout << a << " & " << b << " == " << (a & b) << std::endl;
    std::cout << a << " | " << b << " == " << (a | b) << std::endl;
    std::cout << a << " ^ " << b << " == " << (a ^ b) << std::endl;
    std::cout << b << " & " << a << " == " << (b & a) << std::endl;
    std::cout << b << " | " << a << " == " << (b | a) << std::endl;
    std::cout << b << " ^ " << a << " == " << (b ^ a) << std::endl;
    std::cout << "~" << a << " == " << ~a << std::endl;
    std::cout << a << " ^ " << ~a << " == " << (a ^ ~a) << std::endl;

    std::cout << std::dec;
    std::cout << a << " & " << b << " == " << (a & b) << std::endl;
    std::cout << a << " | " << b << " == " << (a | b) << std::endl;
    std::cout << a << " ^ " << b << " == " << (a ^ b) << std::endl;
    std::cout << b << " & " << a << " == " << (b & a) << std::endl;
    std::cout << b << " | " << a << " == " << (b | a) << std::endl;
    std::cout << b << " ^ " << a << " == " << (b ^ a) << std::endl;
    std::cout << "~" << a << " == " << ~a << std::endl;
    std::cout << a << " ^ " << ~a << " == " << (a ^ ~a) << std::endl;

    for (size_t i = 0; i <= 32; ++i) {
        // randomly test half, but border is mandatory
        if ((std::rand() & 1) && i % 8) continue;
        std::cout << std::hex << std::showbase;
        std::cout << a << " << " << i << " == " << (a << i) << std::endl;
        std::cout << a << " >> " << i << " == " << (a >> i) << std::endl;

        std::cout << std::dec;
        std::cout << a << " << " << i << " == " << (a << i) << std::endl;
        std::cout << a << " >> " << i << " == " << (a >> i) << std::endl;
    }
}
int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    calc::BigInt<> a(1);
    calc::BigInt<> b(2);
    std::srand(static_cast<unsigned>(time(nullptr)));
    do_test(a.GenRandom(3), b.GenRandom(7));
    std::cout << std::endl;
    do_test(a.GenRandom(3).ToOpposite(), b.GenRandom(7));
    std::cout << std::endl;
    do_test(a.GenRandom(3), b.GenRandom(7).ToOpposite());
    std::cout << std::endl;
    do_test(a.GenRandom(3).ToOpposite(), b.GenRandom(7).ToOpposite());
    std::cout << std::endl;
    // std::cout << "----UNSIGNED----" << std::endl;
    // a.is_signed_ = false;
    // b.is_signed_ = false;
    // do_test(a.GenRandom(3), b.GenRandom(7).ToOpposite());
    return 0;
}

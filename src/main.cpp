#include <vector>
#include <iostream>

#include "mos6502.hpp"

#ifdef TEST
#include <cstdint>
#include <iostream>
#include "../test/test.hpp"

#define DEFAULT         "\033[0m"
#define RED             "\033[31m"
#define GREEN           "\033[32m"
#define YELLOW          "\033[33m"

void run_tests() {
    int total_tests = 0;
    int tests_succeeded = 0;
    std::cout << "lda tests:" << std::endl;
    tests_succeeded += test_lda_immediate_load_state();
    tests_succeeded += test_lda_zero_flag();
    total_tests += 2;

    std::cout << std::endl << "tax tests:" << std::endl;
    tests_succeeded += test_tax_load_state();
    tests_succeeded += test_tax_zero_flag();
    total_tests += 2;

    std::cout << std::endl << "inx tests:" << std::endl;
    tests_succeeded += test_inx();
    tests_succeeded += test_inx_overflow();
    total_tests += 2;

    std::cout << std::endl << "iny tests:" << std::endl;
    tests_succeeded += test_iny();
    tests_succeeded += test_iny_overflow();
    total_tests += 2;

    std::cout << YELLOW << "[INFO] " << DEFAULT 
              << tests_succeeded << "/" << total_tests 
              << " ran succesfully." << std::endl;
}
#endif

int main() {
    #ifdef TEST
    run_tests();
    #endif

    CPU nes_6502 = CPU();
    std::vector<uint8_t> game = {
        0x20, 0x06, 0x06, 0x20, 0x38, 0x06, 0x20, 0x0D, 0x06, 0x20, 0x2A, 0x06, 0x60, 0xA9, 0x02, 0x85,
        0x02, 0xA9, 0x04, 0x85, 0x03, 0xA9, 0x11, 0x85, 0x10, 0xA9, 0x10, 0x85, 0x12, 0xA9, 0x0F, 0x85,
        0x14, 0xA9, 0x04, 0x85, 0x11, 0x85, 0x13, 0x85, 0x15, 0x60, 0xA5, 0xFE, 0x85, 0x00, 0xA5, 0xFE,
        0x29, 0x03, 0x18, 0x69, 0x02, 0x85, 0x01, 0x60, 0x20, 0x4D, 0x06, 0x20, 0x8D, 0x06, 0x20, 0xC3,
        0x06, 0x20, 0x19, 0x07, 0x20, 0x20, 0x07, 0x20, 0x2D, 0x07, 0x4C, 0x38, 0x06, 0xA5, 0xFF, 0xC9,
        0x77, 0xF0, 0x0D, 0xC9, 0x64, 0xF0, 0x14, 0xC9, 0x73, 0xF0, 0x1B, 0xC9, 0x61, 0xF0, 0x22, 0x60,
        0xA9, 0x04, 0x24, 0x02, 0xD0, 0x26, 0xA9, 0x01, 0x85, 0x02, 0x60, 0xA9, 0x08, 0x24, 0x02, 0xD0,
        0x1B, 0xA9, 0x02, 0x85, 0x02, 0x60, 0xA9, 0x01, 0x24, 0x02, 0xD0, 0x10, 0xA9, 0x04, 0x85, 0x02,
        0x60, 0xA9, 0x02, 0x24, 0x02, 0xD0, 0x05, 0xA9, 0x08, 0x85, 0x02, 0x60, 0x60, 0x20, 0x94, 0x06,
        0x20, 0xA8, 0x06, 0x60, 0xA5, 0x00, 0xC5, 0x10, 0xD0, 0x0D, 0xA5, 0x01, 0xC5, 0x11, 0xD0, 0x07,
        0xE6, 0x03, 0xE6, 0x03, 0x20, 0x2A, 0x06, 0x60, 0xA2, 0x02, 0xB5, 0x10, 0xC5, 0x10, 0xD0, 0x06,
        0xB5, 0x11, 0xC5, 0x11, 0xF0, 0x09, 0xE8, 0xE8, 0xE4, 0x03, 0xF0, 0x06, 0x4C, 0xAA, 0x06, 0x4C,
        0x35, 0x07, 0x60, 0xA6, 0x03, 0xCA, 0x8A, 0xB5, 0x10, 0x95, 0x12, 0xCA, 0x10, 0xF9, 0xA5, 0x02,
        0x4A, 0xB0, 0x09, 0x4A, 0xB0, 0x19, 0x4A, 0xB0, 0x1F, 0x4A, 0xB0, 0x2F, 0xA5, 0x10, 0x38, 0xE9,
        0x20, 0x85, 0x10, 0x90, 0x01, 0x60, 0xC6, 0x11, 0xA9, 0x01, 0xC5, 0x11, 0xF0, 0x28, 0x60, 0xE6,
        0x10, 0xA9, 0x1F, 0x24, 0x10, 0xF0, 0x1F, 0x60, 0xA5, 0x10, 0x18, 0x69, 0x20, 0x85, 0x10, 0xB0,
        0x01, 0x60, 0xE6, 0x11, 0xA9, 0x06, 0xC5, 0x11, 0xF0, 0x0C, 0x60, 0xC6, 0x10, 0xA5, 0x10, 0x29,
        0x1F, 0xC9, 0x1F, 0xF0, 0x01, 0x60, 0x4C, 0x35, 0x07, 0xA0, 0x00, 0xA5, 0xFE, 0x91, 0x00, 0x60,
        0xA6, 0x03, 0xA9, 0x00, 0x81, 0x10, 0xA2, 0x00, 0xA9, 0x01, 0x81, 0x10, 0x60, 0xA2, 0x00, 0xEA,
        0xEA, 0xCA, 0xD0, 0xFB, 0x60
    };
    nes_6502.load_program(game);
    nes_6502.reset();
    nes_6502.run();
    // nes_6502.hex_dump();
}

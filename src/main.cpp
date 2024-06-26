#include <vector>
#include <iostream>
#include <curses.h>
#include <termios.h>

#include "mos6502.hpp"

// #define TEST

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
    std::cout << "lda tests:" << std::endl << "----------" << std::endl;
    tests_succeeded += test_lda_immediate_load_state();
    tests_succeeded += test_lda_zero_flag();
    total_tests += 2;

    std::cout << std::endl << "tax tests:" << std::endl << "----------" << std::endl;
    tests_succeeded += test_tax_load_state();
    tests_succeeded += test_tax_zero_flag();
    total_tests += 2;

    std::cout << std::endl << "inx tests:" << std::endl << "----------" << std::endl;
    tests_succeeded += test_inx();
    tests_succeeded += test_inx_overflow();
    total_tests += 2;

    std::cout << std::endl << "iny tests:" << std::endl << "----------" << std::endl;
    tests_succeeded += test_iny();
    tests_succeeded += test_iny_overflow();
    total_tests += 2;

    std::cout << std::endl << "adc tests:" << std::endl << "----------" << std::endl;
    tests_succeeded += test_adc();
    tests_succeeded += test_adc_status_updates();
    total_tests += 2;

    std::cout << std::endl << "sbc tests:" << std::endl << "----------" << std::endl;
    tests_succeeded += test_sbc_status_updates();
    total_tests += 1;

    std::cout << YELLOW << "[INFO] " << DEFAULT 
              << tests_succeeded << "/" << total_tests 
              << " ran succesfully." << std::endl;
}
#endif

int run_game() {
    std::vector<uint8_t> game = {
        0x20, 0x06, 0x06, 0x20, 0x38, 0x06, 0x20, 0x0d, 0x06, 0x20, 0x2a, 0x06, 0x60, 0xa9, 0x02, 0x85,
        0x02, 0xa9, 0x04, 0x85, 0x03, 0xa9, 0x11, 0x85, 0x10, 0xa9, 0x10, 0x85, 0x12, 0xa9, 0x0f, 0x85,
        0x14, 0xa9, 0x04, 0x85, 0x11, 0x85, 0x13, 0x85, 0x15, 0x60, 0xa5, 0xfe, 0x85, 0x00, 0xa5, 0xfe,
        0x29, 0x03, 0x18, 0x69, 0x02, 0x85, 0x01, 0x60, 0x20, 0x4d, 0x06, 0x20, 0x8d, 0x06, 0x20, 0xc3,
        0x06, 0x20, 0x19, 0x07, 0x20, 0x20, 0x07, 0x20, 0x2d, 0x07, 0x4c, 0x38, 0x06, 0xa5, 0xff, 0xc9,
        0x77, 0xf0, 0x0d, 0xc9, 0x64, 0xf0, 0x14, 0xc9, 0x73, 0xf0, 0x1b, 0xc9, 0x61, 0xf0, 0x22, 0x60,
        0xa9, 0x04, 0x24, 0x02, 0xd0, 0x26, 0xa9, 0x01, 0x85, 0x02, 0x60, 0xa9, 0x08, 0x24, 0x02, 0xd0,
        0x1b, 0xa9, 0x02, 0x85, 0x02, 0x60, 0xa9, 0x01, 0x24, 0x02, 0xd0, 0x10, 0xa9, 0x04, 0x85, 0x02,
        0x60, 0xa9, 0x02, 0x24, 0x02, 0xd0, 0x05, 0xa9, 0x08, 0x85, 0x02, 0x60, 0x60, 0x20, 0x94, 0x06,
        0x20, 0xa8, 0x06, 0x60, 0xa5, 0x00, 0xc5, 0x10, 0xd0, 0x0d, 0xa5, 0x01, 0xc5, 0x11, 0xd0, 0x07,
        0xe6, 0x03, 0xe6, 0x03, 0x20, 0x2a, 0x06, 0x60, 0xa2, 0x02, 0xb5, 0x10, 0xc5, 0x10, 0xd0, 0x06,
        0xb5, 0x11, 0xc5, 0x11, 0xf0, 0x09, 0xe8, 0xe8, 0xe4, 0x03, 0xf0, 0x06, 0x4c, 0xaa, 0x06, 0x4c,
        0x35, 0x07, 0x60, 0xa6, 0x03, 0xca, 0x8a, 0xb5, 0x10, 0x95, 0x12, 0xca, 0x10, 0xf9, 0xa5, 0x02,
        0x4a, 0xb0, 0x09, 0x4a, 0xb0, 0x19, 0x4a, 0xb0, 0x1f, 0x4a, 0xb0, 0x2f, 0xa5, 0x10, 0x38, 0xe9,
        0x20, 0x85, 0x10, 0x90, 0x01, 0x60, 0xc6, 0x11, 0xa9, 0x01, 0xc5, 0x11, 0xf0, 0x28, 0x60, 0xe6,
        0x10, 0xa9, 0x1f, 0x24, 0x10, 0xf0, 0x1f, 0x60, 0xa5, 0x10, 0x18, 0x69, 0x20, 0x85, 0x10, 0xb0,
        0x01, 0x60, 0xe6, 0x11, 0xa9, 0x06, 0xc5, 0x11, 0xf0, 0x0c, 0x60, 0xc6, 0x10, 0xa5, 0x10, 0x29,
        0x1f, 0xc9, 0x1f, 0xf0, 0x01, 0x60, 0x4c, 0x35, 0x07, 0xa0, 0x00, 0xa5, 0xfe, 0x91, 0x00, 0x60,
        0xa6, 0x03, 0xa9, 0x00, 0x81, 0x10, 0xa2, 0x00, 0xa9, 0x01, 0x81, 0x10, 0x60, 0xa2, 0x00, 0xea,
        0xea, 0xca, 0xd0, 0xfb, 0x60
    };

    // std::vector<uint8_t> program = {
    //     0xA9, 0x10, // lda #$10
    //     0x85, 0x17, // sta $17
    //     0xA9, 0xD0, // lda #$D0
    //     0x85, 0x18, // sta $18
    //     0xA2, 0x02, // ldx #$02
    //     0xA9, 0x0F, // lda #$0F
    //     0x81, 0x15, // sta ($15,X)
    //     0xA9, 0x35, // lda #$35
    //     0x85, 0x2A, // sta $2A
    //     0xA9, 0xC2, // lda #$C2
    //     0x85, 0x2B, // sta $2B
    //     0xA9, 0x2F, // lda #$2F
    //     0x8D, 0x38, 0xC2, // sta $C238
    //     0xA0, 0x03, // ldy #$03
    //     0xA9, 0x03, // lda #$2F
    //     0x51, 0x2A, // eor ($2A),Y
    //     0x85, 0x00, // sta $00
    // };


    auto callback = [](CPU* cpu) {
        for (int k = 0x0200; k < 0x0600; k++) {
            uint8_t pixel_color = cpu->memory[k];

            if ((k+1)%32 == 0) {
                std::cout << std::endl;
            }
            if (pixel_color == 0) {
                std::cout << " ";
            } else if (pixel_color == 1) {
                std::cout << "#";
            } else { // This is the apple
                std::cout << "o";
            }
        }
        std::cout << "\033[2J\033[1;1H";
    };

    CPU nes_6502 = CPU();
    nes_6502.load_program(game);
    nes_6502.reset();
    nes_6502.memory_write(0x00FE, 3);
    nes_6502.memory_write(0x00FF, 0x61);
    // nes_6502.run_callback(callback);
    nes_6502.run();

    return 0;
}

int main() {
    #ifdef TEST
    run_tests();
    #endif

    run_game();
}

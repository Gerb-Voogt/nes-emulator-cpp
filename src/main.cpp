#include "6502.hpp"
#include <vector>

#ifdef TEST
#include <cstdint>
#include <iostream>
#include "test.hpp"

void run_tests() {
	std::cout << "lda tests:" << std::endl;
	test_lda_immediate_load_state();
	test_lda_zero_flag();

	std::cout << std::endl << "tax tests:" << std::endl;
	test_tax_load_state();
	test_tax_zero_flag();

	std::cout << std::endl << "inx tests:" << std::endl;
	test_inx();
	test_inx_overflow();
}
#endif

int main() {
	CPU nes_6505 = CPU();

	std::vector<uint8_t> program = {0xA9, 0x00, 0xAA, 0xE9, 0x00}; // Move 0x00 into register x and increment by 1
	nes_6505.load_program(program);
	#ifdef TEST
	run_tests();
	#endif
}

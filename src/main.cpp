#include "6502.hpp"
#include <vector>

#ifdef TEST
#include <cstdint>
#include <iostream>
#include "../test/test.hpp"

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

void test(void* array) {
}


int main() {
	CPU nes_6505 = CPU();

	#ifdef TEST
	run_tests();
    #endif
}

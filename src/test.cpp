#include <cstdint>
#include <cassert>
#include <iostream>
#include <vector>
#include "6502.hpp"

void test_lda_immediate_load_state() {
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0x05, 0x00};
	cpu.interpret(program);

	assert(cpu.register_a == 0x05);
	assert((cpu.status & 0b00000010) == 0);
	assert((cpu.status & 0b10000000) == 0);

	std::cout << __FUNCTION__ << ": All tests passed" << std::endl;
}

void test_lda_zero_flag() {
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0x00, 0x00};
	cpu.interpret(program);

	assert(cpu.register_a == 0x00);
	assert((cpu.status & 0b00000010) == 0b10);

	std::cout << __FUNCTION__ << ": All tests passed" << std::endl;

}

void test_tax_load_state() {
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0x05, 0xAA, 0x00};
	cpu.interpret(program);

	assert(cpu.register_irx == 0x05);
	assert((cpu.status & 0b00000010) == 0);
	assert((cpu.status & 0b10000000) == 0);

	std::cout << __FUNCTION__ << ": All tests passed" << std::endl;

}

void test_tax_zero_flag() {
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0x00, 0xAA, 0x00};
	cpu.interpret(program);

	assert(cpu.register_irx == 0x00);
	assert((cpu.status & 0b00000010) == 0b10);

	std::cout << __FUNCTION__ << ": All tests passed" << std::endl;
}

void test_inx() {
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0x00, 0xAA, 0xE9, 0x00}; // Move 0x00 into register x and increment by 1
	cpu.interpret(program);

	assert(cpu.register_irx == 0x01);
	assert((cpu.status & 0b00000010) == 0); // N flag should not be set

	std::cout << __FUNCTION__ << ": All tests passed" << std::endl;
}

void test_inx_overflow() {
	CPU cpu = CPU();
	cpu.register_irx = 0xFF; 
	std::vector<uint8_t> program = {0xE9, 0xE9, 0x00};  // Increment irx twice
	cpu.interpret(program);

	assert(cpu.register_irx == 0x01);
	std::cout << __FUNCTION__ << ": All tests passed" << std::endl;
}

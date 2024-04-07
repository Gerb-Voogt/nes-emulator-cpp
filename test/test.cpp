// [TODO]: Implement tests for the following OPCODES:
//		- ADC (Add With Carry)
//		- AND (Logical AND)
//		- ASL (Arithmatic Shift Left)
//		- BCC (Branch if Carry Clear)
//		- BCS (Branch if Carry Set)
//		- INY (Increment Y register), Mostly the same as increment X
//------------------------------------------------------------------------
#include <cstdint>
#include <cassert>
#include <iostream>
#include <vector>
#include "../src/mos6502.hpp"

void test_lda_immediate_load_state() {
	// Test LDA immediate mode
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0x05, 0x00};
	cpu.load_program(program);
	cpu.interpret(program);

	assert(cpu.register_a == 0x05);
	assert((cpu.status & 0b00000010) == 0);
	assert((cpu.status & 0b10000000) == 0);

	std::cout << __FUNCTION__ << ": All tests passed" << std::endl;
}

void test_lda_zero_flag() {
	// Test whether the LDA instruction properly sets the zero flag
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0x00, 0x00};
	cpu.load_program(program);
	cpu.interpret(program);

	assert(cpu.register_a == 0x00);
	assert((cpu.status & 0b00000010) == 0b10);

	std::cout << __FUNCTION__ << ": All tests passed" << std::endl;

}

void test_tax_load_state() {
	// Test whether the TAX instruction properly transfers accumulator content to the X register
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0x05, 0xAA, 0x00};
	cpu.load_program(program);
	cpu.interpret(program);

	assert(cpu.register_irx == 0x05);
	assert((cpu.status & Flag::Zero) == 0);
	assert((cpu.status & Flag::Negative) == 0);

	std::cout << __FUNCTION__ << ": All tests passed" << std::endl;
}

void test_tax_zero_flag() {
	// Test whether the TAX instruction properly sets the 0 flag
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0x00, 0xAA, 0x00};
	cpu.load_program(program);
	cpu.interpret(program);

	assert(cpu.register_irx == 0x00);
	assert((cpu.status & 0b00000010) == 0b10);

	std::cout << __FUNCTION__ << ": All tests passed" << std::endl;
}

void test_inx() {
	// Test whether the INX instruction works as intended
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0x00, 0xAA, 0xE9, 0x00}; // Move 0x00 into register x and increment by 1
	cpu.load_program(program);
	cpu.interpret(program);

	assert(cpu.register_irx == 0x01);
	assert((cpu.status & Flag::Zero) == 0); // N flag should not be set

	std::cout << __FUNCTION__ << ": All tests passed" << std::endl;
}

void test_inx_overflow() {
	// Test whether the INX instruction properly handles the overflow case
	CPU cpu = CPU();
	cpu.register_irx = 0xFF; 
	std::vector<uint8_t> program = {0xE9, 0xE9, 0x00};  // Increment irx twice to overflow
	cpu.load_program(program);
	cpu.interpret(program);

	assert(cpu.register_irx == 0x01);
	std::cout << __FUNCTION__ << ": All tests passed" << std::endl;
}

void test_iny() {
	// Test whether the INY instruction works as intended
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0x00, 0xAA, 0xE9, 0x00}; // Move 0x00 into register x and increment by 1
	cpu.load_program(program);
	cpu.interpret(program);

	assert(cpu.register_irx == 0x01);
	assert((cpu.status & Flag::Zero) == 0); // N flag should not be set

	std::cout << __FUNCTION__ << ": All tests passed" << std::endl;
}

void test_iny_overflow() {
	// Test whether the INY instruction properly handles the overflow case
	CPU cpu = CPU();
	cpu.register_irx = 0xFF; 
	std::vector<uint8_t> program = {0xE9, 0xE9, 0x00};  // Increment irx twice to overflow
	cpu.load_program(program);
	cpu.interpret(program);

	assert(cpu.register_irx == 0x01);
	std::cout << __FUNCTION__ << ": All tests passed" << std::endl;
}

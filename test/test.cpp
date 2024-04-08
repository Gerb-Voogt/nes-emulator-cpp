// [TODO]: Implement tests for the following OPCODES:
//		- ADC (Add With Carry)
//		- AND (Logical AND)
//		- ASL (Arithmatic Shift Left)
//		- BCC (Branch if Carry Clear)
//		- BCS (Branch if Carry Set)
//		- INY (Increment Y register), Mostly the same as increment X
//------------------------------------------------------------------------
#include "test.hpp"
#include <cstdint>
#include <iostream>
#include <vector>
#include "../src/mos6502.hpp"

#define DEFAULT         "\033[0m"
#define RED             "\033[31m"
#define GREEN           "\033[32m"
#define YELLOW          "\033[33m"

int test_lda_immediate_load_state() {
	// Test LDA immediate mode
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0x05, 0x00};
	cpu.load_program(program);
	cpu.reset();
	cpu.run();

	if (cpu.register_a != 0x05) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.register_a != 0x05"
				  << std::endl;
		return 0;
	}
	if ((cpu.status & Flag::Zero) != 0){ 
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.status & Flag::Zero != 0"
				  << std::endl;
		return 0;
	}
	if ((cpu.status & Flag::Negative) != 0) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.status & Flag::Negative != 0"
				  << std::endl;
		return 0;
	}

	std::cout << GREEN << "[SUCCESS]: " << DEFAULT 
		      << __FUNCTION__ << ": All tests passed" << std::endl;
	return 1;
}

int test_lda_zero_flag() {
	// Test whether the LDA instruction properly sets the zero flag
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0x00, 0x00};
	cpu.load_program(program);
	cpu.reset();
	cpu.run();

	if (cpu.register_a != 0x00) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.register_a != 0x00"
				  << std::endl;
		return 0;
	}
	if ((cpu.status & Flag::Zero) != 0b10) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.status & Flag::Zero != 0b00000010"
				  << std::endl;
		return 0;
	}

	std::cout << GREEN << "[SUCCESS]: " << DEFAULT 
		      << __FUNCTION__ << ": All tests passed" << std::endl;
	return 1;
}

int test_tax_load_state() {
	// Test whether the TAX instruction properly transfers accumulator content to the X register
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0x05, 0xAA, 0x00};
	cpu.load_program(program);
	cpu.reset();
	cpu.run();

	if (cpu.register_irx != 0x05) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.register_a != 0x00"
				  << std::endl;
		return 0;
	}
	if ((cpu.status & Flag::Zero) != 0){
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.status & Flag::Zero != 0"
				  << std::endl;
		return 0;
	}
	if ((cpu.status & Flag::Negative) != 0) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.status & Flag::Negative != 0"
				  << std::endl;
		return 0;
	}

	std::cout << GREEN << "[SUCCESS]: " << DEFAULT 
		      << __FUNCTION__ << ": All tests passed" << std::endl;
	return 1;
}

int test_tax_zero_flag() {
	// Test whether the TAX instruction properly sets the 0 flag
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0x00, 0xAA, 0x00};
	cpu.load_program(program);
	cpu.reset();
	cpu.run();

	if (cpu.register_irx != 0x00) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.register_irx != 0x00"
				  << std::endl;
		return 0;
	}
	if ((cpu.status & Flag::Zero) != 0b10) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.status & Flag::Zero != 0"
				  << std::endl;
		return 0;
	}

	std::cout << GREEN << "[SUCCESS]: " << DEFAULT 
		      << __FUNCTION__ << ": All tests passed" << std::endl;
	return 1;
}

int test_inx() {
	// Test whether the INX instruction works as intended
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0x00, 0xAA, 0xE8, 0x00}; // Move 0x00 into register x and increment by 1
	cpu.load_program(program);
	cpu.reset();
	cpu.run();

	if (cpu.register_irx != 0x01) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.register_irx != 0x01"
				  << std::endl;
		return 0;
	}
	if ((cpu.status & Flag::Zero) != 0) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.status & Flag::Zero != 0"
				  << std::endl;
		// N flag should not be set
		return 0;
	}

	std::cout << GREEN << "[SUCCESS]: " << DEFAULT 
		      << __FUNCTION__ << ": All tests passed" << std::endl;
	return 1;
}

int test_inx_overflow() {
	// Test whether the INX instruction properly handles the overflow case
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA2, 0xFF, 0xE8, 0xE8, 0x00};  // Increment irx twice to overflow
	cpu.load_program(program);
	cpu.reset();
	cpu.run();

	if (cpu.register_irx != 0x01) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.register_irx != 0x01"
				  << std::endl;
		return 0;
	}
	std::cout << GREEN << "[SUCCESS]: " << DEFAULT 
		      << __FUNCTION__ << ": All tests passed" << std::endl;
	return 1;
}

int test_iny() {
	// Test whether the INY instruction works as intended
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0x00, 0xA8, 0xC8, 0x00}; // Move 0x00 into register x and increment by 1
	cpu.load_program(program);
	cpu.reset();
	cpu.run();

	if (cpu.register_iry != 0x01) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.register_iry != 0x01"
				  << std::endl;
		return 0;
	}
	if ((cpu.status & Flag::Zero) != 0) {
		// N flag should not be set
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.status & Flag::Zero != 0x01"
				  << std::endl;
		return 0;
	}

	std::cout << GREEN << "[SUCCESS]: " << DEFAULT 
		      << __FUNCTION__ << ": All tests passed" << std::endl;
	return 1;
}

int test_iny_overflow() {
	// Test whether the INY instruction properly handles the overflow case
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0xFF, 0xA8, 0xC8, 0xC8, 0x00};  // Increment irx twice to overflow
	cpu.load_program(program);
	cpu.reset();
	cpu.run();

	if (cpu.register_iry != 0x01) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.register_iry != 0x01"
				  << std::endl;
		return 0;
	}

	std::cout << GREEN << "[SUCCESS]: " << DEFAULT 
		      << __FUNCTION__ << ": All tests passed" << std::endl;
	return 1;
}

int test_adc() {
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0x12, 0x69, 0x22, 0x00};
	cpu.load_program(program);
	cpu.reset();
	cpu.run();

	if (cpu.register_a != (0x12 + 0x22)) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.register_iry != 0x01"
				  << std::endl;
		return 0;
	}

	std::vector<uint8_t> program_with_carry_bit = {0x38, 0xA9, 0x12, 0x69, 0x22, 0x00};
	cpu.load_program(program_with_carry_bit);
	cpu.reset();
	cpu.run();

	if (cpu.register_a != (0x12 + 0x22 + 1)) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.register_iry != 0x01"
				  << std::endl;
		return 0;
	}

	std::cout << GREEN << "[SUCCESS]: " << DEFAULT 
		      << __FUNCTION__ << ": All tests passed" << std::endl;
	return 1;
}

int test_adc_status_updates() {
	CPU cpu = CPU();
	std::vector<uint8_t> program = {0xA9, 0xFF, 0x69, 0x22, 0x00};
	cpu.load_program(program);
	cpu.reset();
	cpu.run();

	if (cpu.register_a != (0x21) || (cpu.status & Flag::Carry) == 0) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.register_a != 0x22, cpu.status & Flag::Carry == 0"
				  << std::endl;
		return 0;
	}

	cpu.reset_memory_space();
	std::vector<uint8_t> program_new = {0xA9, 0xFF, 0x69, 0x22, 0x69, 0x22, 0x00};
	cpu.load_program(program_new);
	cpu.reset();
	cpu.run();

	if (cpu.register_a != (0x21 + 0x22 + 1) || (cpu.status & Flag::Carry) != 0) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.register_a != 0x44, cpu.status & Flag::Carry != 0"
				  << std::endl;
		return 0;
	}

	std::cout << GREEN << "[SUCCESS]: " << DEFAULT 
		      << __FUNCTION__ << ": All tests passed" << std::endl;
	return 1;
}

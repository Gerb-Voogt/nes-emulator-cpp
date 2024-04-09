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
			      << __FUNCTION__ << ": cpu.register_a != 0x12 + 0x22"
				  << std::endl;
		return 0;
	}

	std::vector<uint8_t> program_with_carry_bit = {0x38, 0xA9, 0x12, 0x69, 0x22, 0x00};
	cpu.load_program(program_with_carry_bit);
	cpu.reset();
	cpu.run();

	if (cpu.register_a != (0x12 + 0x22 + 1)) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": cpu.register_a != 0x01"
				  << std::endl;
		return 0;
	}

	std::cout << GREEN << "[SUCCESS]: " << DEFAULT 
		      << __FUNCTION__ << ": All tests passed" << std::endl;
	return 1;
}

int test_adc_status_updates() {
	/*
	 * ; Program: ;
	 * ; Program does some ADC operation and then pushes the status register to the stack.
	 *
	 * ; V and C Flags should both be clear ;
	 * LDA #$50
	 * ADC #$10
	 * STA $00  ; Should be 0x60
	 * PHP
	 * CLC
	 * CLV
	 *
	 * ; V flag and C Flags should both be set ;
	 * LDA #$D0;
	 * ADC #$90;
	 * STA $01  ; Should be 0x60
	 * PHP
	 * CLC
	 * CLV
	 *
	 * ; V flag Should be set, C Flag should be clear
	 * LDA #$50;
	 * ADC #$50;
	 * STA $02  ; Should be 0xA0
	 * PHP
	 * CLC
	 * CLV
	 *
	 * ; Check the content of the stack (for all 3 situations to check whethter the flags where set correctly.
	 */
	CPU cpu = CPU();
	std::vector<uint8_t> program = {
		0xA9, 0x50, // LDA #$50
		0x69, 0x10, // ADC #$10
		0x85, 0x00, // STA $00
		0x08, 0x18, 0xB8, // PHP, CLC, CLV
		0xA9, 0xD0, // LDA #$50
		0x69, 0x90, // ADC #$10
		0x85, 0x01, // STA $00
		0x08, 0x18, 0xB8, // PHP, CLC, CLV
		0xA9, 0x50, // LDA #$50
		0x69, 0x50, // ADC #$50
		0x85, 0x02, // STA $00
		0x08, 0x18, 0xB8, // PHP, CLC, CLV
		0x00
	};
	cpu.load_program(program);
	cpu.reset();
	cpu.run();
    // cpu.hex_dump_zero_page();
    // cpu.hex_dump_stack();

    // Pop in reverse order, stack is LIFO
    uint8_t status_3 = cpu.pop_stack();
    uint8_t status_2 = cpu.pop_stack();
	uint8_t status_1 = cpu.pop_stack();
	uint8_t res_1 = cpu.memory_read(0x0000);
	uint8_t res_2 = cpu.memory_read(0x0001);
	uint8_t res_3 = cpu.memory_read(0x0002);

    // First Test
    if ((status_1 & (Flag::Overflow | Flag::Carry)) != 0) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": status_1 & (Flag::Overflow | Flag::Carry) == 0" << std::endl
                  << "Overflow and Carry Flags not set correctly" << std::endl;
		return 0;
    }
    if (res_1 != 0x60) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": res_1 != 0x60" << std::endl
                  << "Result 1 Incorrect" << std::endl;
		return 0;
    }

    // Second Test
    if ((status_2 & (Flag::Overflow | Flag::Carry)) != (Flag::Overflow | Flag::Carry)) {
        std::cout << RED << "[FAIL]: " << DEFAULT
                  << __FUNCTION__ << ": (status_2 & (Flag::Overflow | Flag::Carry)) != (Flag::Overflow | Flag::Carry)" << std::endl
                  << "Overflow and Carry Flags not set correctly" << std::endl;
        return 0;
    }
    if (res_2 != 0x60) {
        std::cout << RED << "[FAIL]: " << DEFAULT
                  << __FUNCTION__ << ": res_2 != 0x60" << std::endl
                  << "Result 1 Incorrect" << std::endl;
        return 0;
    }

    // Third Test
    if ((status_3 & Flag::Overflow) == 0 || (status_3 & Flag::Carry) != 0) {
        // If the Overflow Flag is NOT set OR the Carry Flag IS set
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": (status_3 & Flag::Overflow) == 0 || (status_3 & Flag::Carry) != 0" << std::endl
                  << "Overflow and Carry Flags not set correctly" << std::endl;
		return 0;
    }
    if (res_3 != 0xA0) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": res_3 != 0xA0" << std::endl
                  << "Result 1 Incorrect" << std::endl;
		return 0;
    }

	std::cout << GREEN << "[SUCCESS]: " << DEFAULT 
		      << __FUNCTION__ << ": All tests passed" << std::endl;
	return 1;
}


int test_sbc_status_updates() {
	/*
	 * ; Program: ;
	 * ; Program does some ADC operation and then pushes the status register to the stack.
	 *
	 * ; V flag should be clear and C Flag should be set ;
	 * LDA #$50
	 * SBC #$F0
	 * STA $00  ; Should be 0x60
	 * PHP
	 * CLC
	 * CLV
	 *
	 * ; V and C flags should both be set
	 * LDA #$50;
	 * SBC #$B0;
	 * STA $01  ; Should be 0xA0
	 * PHP
	 * CLC
	 * CLV
	 *
	 * ; V flag Should be set, C Flag should be clear
	 * LDA #$D0;
	 * SBC #$70;
	 * STA $02  ; Should be 0x60
	 * PHP
	 * CLC
	 * CLV
	 *
	 * ; Check the content of the stack (for all 3 situations to check whethter the flags where set correctly.
	 */
	CPU cpu = CPU();
	std::vector<uint8_t> program = {
		0xA9, 0x50, // LDA #$50
		0xE9, 0xF0, // SBC #$F0
		0x85, 0x00, // STA $00
		0x08, 0x18, 0xB8, // PHP, CLC, CLV
		0xA9, 0x50, // LDA #$50
		0xE9, 0xB0, // SBC #$B0
		0x85, 0x01, // STA $01
		0x08, 0x18, 0xB8, // PHP, CLC, CLV
		0xA9, 0xD0, // LDA #$D0
		0xE9, 0x70, // SBC #$70
		0x85, 0x02, // STA $02
		0x08, 0x18, 0xB8, // PHP, CLC, CLV
		0x00
	};
	cpu.load_program(program);
	cpu.reset();
	cpu.run();
    // cpu.hex_dump_zero_page();
    // cpu.hex_dump_stack();

    // Pop in reverse order, stack is LIFO
    uint8_t status_3 = cpu.pop_stack();
    uint8_t status_2 = cpu.pop_stack();
	uint8_t status_1 = cpu.pop_stack();
	uint8_t res_1 = cpu.memory_read(0x0000);
	uint8_t res_2 = cpu.memory_read(0x0001);
	uint8_t res_3 = cpu.memory_read(0x0002);

    // First Test
    if ((status_1 & Flag::Overflow) != 0 || (status_1 & Flag::Carry) == 0) {
        // If the Overflow Flag is NOT set OR the Carry Flag IS set
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": status_1 & (Flag::Overflow | Flag::Carry) == 0" << std::endl
                  << "Overflow and Carry Flags not set correctly" << std::endl;
		return 0;
    }
    if (res_1 != 0x60) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": res_1 != 0x60" << std::endl
                  << "Result 1 Incorrect" << std::endl;
		return 0;
    }

    // Second Test
    if ((status_2 & (Flag::Overflow | Flag::Carry)) != (Flag::Overflow | Flag::Carry)) {
        std::cout << RED << "[FAIL]: " << DEFAULT
                  << __FUNCTION__ << ": (status_2 & (Flag::Overflow | Flag::Carry)) != (Flag::Overflow | Flag::Carry)" << std::endl
                  << "Overflow and Carry Flags not set correctly" << std::endl;
        return 0;
    }
    if (res_2 != 0xA0) {
        std::cout << RED << "[FAIL]: " << DEFAULT
                  << __FUNCTION__ << ": res_2 != 0x60" << std::endl
                  << "Result 1 Incorrect" << std::endl;
        return 0;
    }

    // Third Test
    if ((status_3 & Flag::Overflow) == 0 || (status_3 & Flag::Carry) != 0) {
        // If the Overflow Flag is NOT set OR the Carry Flag IS set
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": (status_3 & Flag::Overflow) == 0 || (status_3 & Flag::Carry) != 0" << std::endl
                  << "Overflow and Carry Flags not set correctly" << std::endl;
		return 0;
    }
    if (res_3 != 0x60) {
		std::cout << RED << "[FAIL]: " << DEFAULT
			      << __FUNCTION__ << ": res_3 != 0xA0" << std::endl
                  << "Result 1 Incorrect" << std::endl;
		return 0;
    }

	std::cout << GREEN << "[SUCCESS]: " << DEFAULT 
		      << __FUNCTION__ << ": All tests passed" << std::endl;
	return 1;
}

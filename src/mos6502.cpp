#include <bitset>
#include <chrono>
#include <cstdint>
#include <functional>
#include <ios>
#include <ostream>
#include <stdexcept>
#include <thread>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>

#include "mos6502.hpp"

// Implement the OPCODE table later and integrate this
// I think this warrants the use of a new, seperate cpp file.
// Either that or move this to the bottom of the file
std::map<int, Opcode> OPCODES = {
	// {id, Opcode(id, bytes, cycles, addressingmode, mnenomic}
	{0x69, Opcode(0x69, 2, 3, AddressingMode::Immediate, "adc")},
	{0x65, Opcode(0x65, 2, 3, AddressingMode::ZeroPage, "adc")},
	{0x75, Opcode(0x75, 2, 4, AddressingMode::ZeroPageX, "adc")},
	{0x6D, Opcode(0x6D, 3, 4, AddressingMode::Absolute, "adc")},
	{0x7D, Opcode(0x7D, 3, 4, AddressingMode::AbsoluteX, "adc")}, // cycles + 1 if page crossed
	{0x79, Opcode(0x79, 3, 4, AddressingMode::AbsoluteY, "adc")}, // cycles + 1 if page crossed
	{0x61, Opcode(0x61, 2, 6, AddressingMode::IndirectX, "adc")},
	{0x71, Opcode(0x79, 2, 5, AddressingMode::IndirectY, "adc")}, // cycles + 1 if page crossed

	{0x29, Opcode(0x29, 2, 2, AddressingMode::Immediate, "and")},
	{0x25, Opcode(0x25, 2, 3, AddressingMode::ZeroPage, "and")},
	{0x35, Opcode(0x35, 2, 4, AddressingMode::ZeroPageX, "and")},
	{0x2D, Opcode(0x2D, 3, 4, AddressingMode::Absolute, "and")},
	{0x3D, Opcode(0x3D, 3, 4, AddressingMode::AbsoluteX, "and")}, // cycles + 1 if page crossed
	{0x39, Opcode(0x39, 3, 4, AddressingMode::AbsoluteY, "and")}, // cycles + 1 if page crossed
	{0x21, Opcode(0x21, 2, 6, AddressingMode::IndirectX, "and")},
	{0x31, Opcode(0x31, 2, 5, AddressingMode::IndirectY, "and")}, // cycles + 1 if page crossed

	{0x0A, Opcode(0x0A, 1, 2, AddressingMode::Accumulator, "asl")},
	{0x06, Opcode(0x06, 2, 5, AddressingMode::ZeroPage, "asl")},
	{0x16, Opcode(0x16, 2, 6, AddressingMode::ZeroPageX, "asl")},
	{0x0E, Opcode(0x0E, 3, 6, AddressingMode::Absolute, "asl")},
	{0x1E, Opcode(0x1E, 3, 7, AddressingMode::AbsoluteX, "asl")},

	{0x24, Opcode(0x24, 2, 3, AddressingMode::ZeroPage, "bit")},
	{0x2C, Opcode(0x2C, 3, 4, AddressingMode::Absolute, "bit")},

	{0x00, Opcode(0x00, 1, 7, AddressingMode::Implied, "brk")},

	{0xC9, Opcode(0xC9, 2, 2, AddressingMode::Immediate, "cmp")},
	{0xC5, Opcode(0xC5, 2, 3, AddressingMode::ZeroPage, "cmp")},
	{0xD5, Opcode(0xD5, 2, 4, AddressingMode::ZeroPageX, "cmp")},
	{0xCD, Opcode(0xCD, 3, 4, AddressingMode::Absolute, "cmp")},
	{0xDD, Opcode(0xDD, 3, 4, AddressingMode::AbsoluteX, "cmp")}, // cycles + 1 if page crossed
	{0xD9, Opcode(0xD9, 3, 4, AddressingMode::AbsoluteY, "cmp")}, // cycles + 1 if page crossed
	{0xC1, Opcode(0xC1, 2, 6, AddressingMode::IndirectX, "cmp")},
	{0xD1, Opcode(0xD1, 2, 5, AddressingMode::IndirectY, "cmp")}, // cycles + 1 if page crossed

	{0xE0, Opcode(0xE0, 2, 2, AddressingMode::Immediate, "cpx")},
	{0xE4, Opcode(0xE4, 2, 3, AddressingMode::ZeroPage, "cpx")},
	{0xEC, Opcode(0xEC, 3, 4, AddressingMode::Absolute, "cpx")},

	{0xC0, Opcode(0xC0, 2, 2, AddressingMode::Immediate, "cpy")},
	{0xC4, Opcode(0xC4, 2, 3, AddressingMode::ZeroPage, "cpy")},
	{0xCC, Opcode(0xCC, 3, 4, AddressingMode::Absolute, "cpy")},

	{0xC6, Opcode(0xC6, 2, 5, AddressingMode::ZeroPage, "dec")},
	{0xD6, Opcode(0xD6, 2, 6, AddressingMode::ZeroPageX, "dec")},
	{0xCE, Opcode(0xCE, 3, 6, AddressingMode::Absolute, "dec")},
	{0xDE, Opcode(0xDE, 3, 7, AddressingMode::AbsoluteX, "dec")},

	{0xCA, Opcode(0xCA, 1, 2, AddressingMode::Implied, "dex")},
	{0x88, Opcode(0x88, 1, 2, AddressingMode::Implied, "dey")},

	{0x49, Opcode(0x49, 2, 2, AddressingMode::Immediate, "eor")},
	{0x45, Opcode(0x45, 2, 3, AddressingMode::ZeroPage, "eor")},
	{0x55, Opcode(0x55, 2, 4, AddressingMode::ZeroPageX, "eor")},
	{0x4D, Opcode(0x4D, 3, 4, AddressingMode::Absolute, "eor")},
	{0x5D, Opcode(0x5D, 3, 4, AddressingMode::AbsoluteX, "eor")}, // cycles + 1 if page crossed
	{0x59, Opcode(0x59, 3, 4, AddressingMode::AbsoluteY, "eor")}, // cycles + 1 if page crossed
	{0x41, Opcode(0x41, 2, 6, AddressingMode::IndirectX, "eor")},
	{0x51, Opcode(0x51, 2, 5, AddressingMode::IndirectY, "eor")}, // cycles + 1 if page crossed
	
	{0xE6, Opcode(0xE6, 2, 5, AddressingMode::ZeroPage, "inc")},
	{0xF6, Opcode(0xF6, 2, 6, AddressingMode::ZeroPageX, "inc")},
	{0xEE, Opcode(0xEE, 3, 6, AddressingMode::Absolute, "inc")},
	{0xFE, Opcode(0xFE, 3, 7, AddressingMode::AbsoluteX, "inc")},

	{0xE8, Opcode(0xE8, 1, 2, AddressingMode::Implied, "inx")},
	{0xC8, Opcode(0xC8, 1, 2, AddressingMode::Implied, "iny")},

	{0x4C, Opcode(0x4C, 3, 3, AddressingMode::Absolute, "jmp")},
	{0x6C, Opcode(0x6C, 3, 3, AddressingMode::Indirect, "jmp")},

	{0x20, Opcode(0x20, 3, 6, AddressingMode::Absolute, "jsr")},

	{0xA9, Opcode(0xA9, 2, 2, AddressingMode::Immediate, "lda")},
	{0xA5, Opcode(0xA5, 2, 3, AddressingMode::ZeroPage, "lda")},
	{0xB5, Opcode(0xB5, 2, 4, AddressingMode::ZeroPageX, "lda")},
	{0xAD, Opcode(0xAD, 3, 4, AddressingMode::Absolute, "lda")},
	{0xBD, Opcode(0xBD, 3, 4, AddressingMode::AbsoluteX, "lda")}, // cycles + 1 if page crossed
	{0xB9, Opcode(0xB9, 3, 4, AddressingMode::AbsoluteY, "lda")}, // cycles + 1 if page crossed
	{0xA1, Opcode(0xA1, 2, 6, AddressingMode::IndirectX, "lda")},
	{0xB1, Opcode(0xB1, 2, 5, AddressingMode::IndirectY, "lda")}, // cycles + 1 if page crossed

	{0xA2, Opcode(0xA2, 2, 2, AddressingMode::Immediate, "ldx")},
	{0xA6, Opcode(0xA6, 2, 3, AddressingMode::ZeroPage, "ldx")},
	{0xB6, Opcode(0xB6, 2, 4, AddressingMode::ZeroPageY, "ldx")},
	{0xAE, Opcode(0xAE, 3, 4, AddressingMode::Absolute, "ldx")},
	{0xBE, Opcode(0xBE, 3, 4, AddressingMode::AbsoluteY, "ldx")}, // cycles + 1 if page crossed

	{0xA0, Opcode(0xA0, 2, 2, AddressingMode::Immediate, "ldy")},
	{0xA4, Opcode(0xA4, 2, 3, AddressingMode::ZeroPage, "ldy")},
	{0xB4, Opcode(0xB4, 2, 4, AddressingMode::ZeroPageX, "ldy")},
	{0xAC, Opcode(0xAC, 3, 4, AddressingMode::Absolute, "ldy")},
	{0xBC, Opcode(0xBC, 3, 4, AddressingMode::AbsoluteX, "ldy")}, // cycles + 1 if page crossed

	{0x4A, Opcode(0x4A, 1, 2, AddressingMode::Accumulator, "lsr")},
	{0x46, Opcode(0x46, 2, 5, AddressingMode::ZeroPage, "lsr")},
	{0x56, Opcode(0x56, 2, 6, AddressingMode::ZeroPageX, "lsr")},
	{0x4E, Opcode(0x4E, 3, 6, AddressingMode::Absolute, "lsr")},
	{0x5E, Opcode(0x5E, 3, 7, AddressingMode::AbsoluteX, "lsr")},

	{0xEA, Opcode(0xEA, 1, 2, AddressingMode::Implied, "nop")},
	
	{0x09, Opcode(0x09, 2, 2, AddressingMode::Immediate, "ora")},
	{0x05, Opcode(0x05, 2, 3, AddressingMode::ZeroPage, "ora")},
	{0x15, Opcode(0x15, 2, 4, AddressingMode::ZeroPageX, "ora")},
	{0x0D, Opcode(0x0D, 3, 4, AddressingMode::Absolute, "ora")},
	{0x1D, Opcode(0x1D, 3, 4, AddressingMode::AbsoluteX, "ora")}, // cycles + 1 if page crossed
	{0x19, Opcode(0x19, 3, 4, AddressingMode::AbsoluteY, "ora")}, // cycles + 1 if page crossed
	{0x01, Opcode(0x01, 2, 6, AddressingMode::IndirectX, "ora")},
	{0x11, Opcode(0x11, 2, 5, AddressingMode::IndirectY, "ora")}, // cycles + 1 if page crossed
	
	{0x2A, Opcode(0x2A, 1, 2, AddressingMode::Accumulator, "rol")},
	{0x26, Opcode(0x26, 2, 5, AddressingMode::ZeroPage, "rol")},
	{0x36, Opcode(0x36, 2, 6, AddressingMode::ZeroPageX, "rol")},
	{0x2E, Opcode(0x2E, 3, 6, AddressingMode::Absolute, "rol")},
	{0x3E, Opcode(0x3E, 3, 7, AddressingMode::AbsoluteX, "rol")},

	{0x6A, Opcode(0x6A, 1, 2, AddressingMode::Accumulator, "ror")},
	{0x66, Opcode(0x66, 2, 5, AddressingMode::ZeroPage, "ror")},
	{0x76, Opcode(0x76, 2, 6, AddressingMode::ZeroPageX, "ror")},
	{0x6E, Opcode(0x6E, 3, 6, AddressingMode::Absolute, "ror")},
	{0x7E, Opcode(0x7E, 3, 7, AddressingMode::AbsoluteX, "ror")},

	{0x40, Opcode(0x40, 1, 6, AddressingMode::Implied, "rti")},
	{0x60, Opcode(0x60, 1, 6, AddressingMode::Implied, "rts")},

	{0xE9, Opcode(0xE9, 2, 2, AddressingMode::Immediate, "sbc")},
	{0xE5, Opcode(0xE5, 2, 3, AddressingMode::ZeroPage, "sbc")},
	{0xF5, Opcode(0xF5, 2, 4, AddressingMode::ZeroPageX, "sbc")},
	{0xED, Opcode(0xED, 3, 4, AddressingMode::Absolute, "sbc")},
	{0xFD, Opcode(0xFD, 3, 4, AddressingMode::AbsoluteX, "sbc")}, // cycles + 1 if page crossed
	{0xF9, Opcode(0xF9, 3, 4, AddressingMode::AbsoluteY, "sbc")}, // cycles + 1 if page crossed
	{0xE1, Opcode(0xE1, 2, 6, AddressingMode::IndirectX, "sbc")},
	{0xF1, Opcode(0xF1, 2, 5, AddressingMode::IndirectY, "sbc")}, // cycles + 1 if page crossed

	{0x85, Opcode(0x85, 2, 3, AddressingMode::ZeroPage, "sta")},
	{0x95, Opcode(0x95, 2, 4, AddressingMode::ZeroPageX, "sta")},
	{0x8D, Opcode(0x8D, 3, 4, AddressingMode::Absolute, "sta")},
	{0x9D, Opcode(0x9D, 3, 4, AddressingMode::AbsoluteX, "sta")},
	{0x99, Opcode(0x99, 3, 5, AddressingMode::AbsoluteY, "sta")},
	{0x81, Opcode(0x81, 3, 5, AddressingMode::IndirectX, "sta")},
	{0x91, Opcode(0x91, 2, 6, AddressingMode::IndirectY, "sta")},

	{0x86, Opcode(0x86, 2, 3, AddressingMode::ZeroPage, "stx")},
	{0x96, Opcode(0x96, 2, 4, AddressingMode::ZeroPageY, "stx")},
	{0x8E, Opcode(0x8E, 3, 4, AddressingMode::Absolute, "stx")},

	{0x84, Opcode(0x84, 2, 3, AddressingMode::ZeroPage, "sty")},
	{0x94, Opcode(0x94, 2, 4, AddressingMode::ZeroPageX, "sty")},
	{0x8C, Opcode(0x8C, 3, 4, AddressingMode::Absolute, "sty")},

	{0xAA, Opcode(0xAA, 1, 2, AddressingMode::Implied, "tax")},
	{0xA8, Opcode(0xA8, 1, 2, AddressingMode::Implied, "tay")},
	{0xBA, Opcode(0xBA, 1, 2, AddressingMode::Implied, "tsx")},
	{0x8A, Opcode(0x8A, 1, 2, AddressingMode::Implied, "txa")},
	{0x9A, Opcode(0x9A, 1, 2, AddressingMode::Implied, "txs")},
	{0x98, Opcode(0x98, 1, 2, AddressingMode::Implied, "tya")},

	// Stack Instructions
	{0x48, Opcode(0x48, 1, 3, AddressingMode::Implied, "pha")},
	{0x08, Opcode(0x08, 1, 3, AddressingMode::Implied, "php")},
	{0x68, Opcode(0x68, 1, 4, AddressingMode::Implied, "pla")},
	{0x28, Opcode(0x28, 1, 4, AddressingMode::Implied, "plp")},

	// Flag instructions
	{0x18, Opcode(0x18, 1, 2, AddressingMode::Implied, "clc")},
	{0xD8, Opcode(0xD8, 1, 2, AddressingMode::Implied, "cld")},
	{0x58, Opcode(0x58, 1, 2, AddressingMode::Implied, "cli")},
	{0xB8, Opcode(0xB8, 1, 2, AddressingMode::Implied, "clv")},
	{0x38, Opcode(0x38, 1, 2, AddressingMode::Implied, "sec")},
	{0xF8, Opcode(0xF8, 1, 2, AddressingMode::Implied, "sed")},
	{0x78, Opcode(0x78, 1, 2, AddressingMode::Implied, "sei")},

	// Branch instructions
	{0x90, Opcode(0x90, 2, 2, AddressingMode::Relative, "bcc")}, // + 1 if branch succeeds, +2 if branch to new page
	{0xB0, Opcode(0xB0, 2, 2, AddressingMode::Relative, "bcs")}, // + 1 if branch succeeds, +2 if branch to new page
	{0xF0, Opcode(0xF0, 2, 2, AddressingMode::Relative, "beq")}, // + 1 if branch succeeds, +2 if branch to new page
	{0x30, Opcode(0x30, 2, 2, AddressingMode::Relative, "bmi")}, // + 1 if branch succeeds, +2 if branch to new page
	{0xD0, Opcode(0xD0, 2, 2, AddressingMode::Relative, "bne")}, // + 1 if branch succeeds, +2 if branch to new page
	{0x10, Opcode(0x10, 2, 2, AddressingMode::Relative, "bpl")}, // + 1 if branch succeeds, +2 if branch to new page
	{0x50, Opcode(0x50, 2, 2, AddressingMode::Relative, "bvc")}, // + 1 if branch succeeds, +2 if branch to new page
	{0x70, Opcode(0x70, 2, 2, AddressingMode::Relative, "bvs")}, // + 1 if branch succeeds, +2 if branch to new page
};


Opcode::Opcode() {
	this->code = 0;
	this->size = 0;
	this->cycles = 0;
	this->mode = AddressingMode::Implied;
	this->name = "";
}


Opcode::Opcode(short code,
			   short size,
			   short cycles,
			   AddressingMode mode,
			   std::string name) {
	this->code = code;
	this->size = size;
	this->cycles = cycles;
	this->mode = mode;
	this->name = name;
}


CPU::CPU() {
	this->register_a = 0;
	this->register_irx = 0;
	this->register_iry = 0;
	this->program_counter = 0;
	this->stack_pointer = 0xFF;
	this->status = 0;
	this->cycles = 0;
	this->cycle_duration = 559; // ns

	// Initialize memory space to 0
	for (int i = 0; i < 0xFFFF; i++) {
		this->memory[i] = 0;
	}
}


uint8_t CPU::memory_read(const uint16_t addr) const {
	return this->memory[addr];
}


uint16_t CPU::memory_read_uint16(const uint16_t addr) const {
	// 8 bit values read into 16 bit variables such that the left most
	// byte for both is padded with zeros
	uint16_t lo_byte = memory_read(addr);
	uint16_t hi_byte = memory_read(addr+1);

	// Left shift hi_byte by 8 such that it is the left-most byte sequence
	// This is to correct for the fact that the 6502 CPU of the NES is little endian
	return ((hi_byte << 8) | lo_byte);
}


void CPU::memory_write(const uint16_t addr, const uint8_t data) {
	this->memory[addr] = data;
}


void CPU::memory_write_uint16(const uint16_t addr, const uint16_t data) {
	uint8_t hi_byte = (data >> 8); // left shift by 8 to get the upper half of data into uint8_t
	uint8_t lo_byte = (data & 0b11111111); // bitwise & with 255 in order to extract the lower half of data

	// lo byte (lower half byte of data) should be written to addr
	// hi byte (upper half byte of data) should be written to addr + 1
	memory_write(addr, lo_byte);
	memory_write(addr+1, hi_byte);
}


void CPU::push_stack(const uint8_t data) {
	if (this->stack_pointer == 0x00) {
		// Wrap the stack pointer if overflow happens
		this->stack_pointer = 0xFF;
	}

	// Get the address and put on the stack
	uint16_t address = 0x0100 + this->stack_pointer;
	memory_write(address, data);

	// Move the stack pointer to the new empty address
	this->stack_pointer -= 1;
}


void CPU::push_stack_uint16(const uint16_t data) {
	uint16_t address_lo_byte = 0x0100 + this->stack_pointer;
	uint16_t address_hi_byte = 0x0100 + this->stack_pointer - 1;

	// Convert data and write to memory
	// hi byte should be written to stack_pointer-1, lo byte to stack_pointer
	uint8_t hi_byte = (data >> 8);
	uint8_t lo_byte = (data & 0xFF);
	memory_write(address_lo_byte, lo_byte);
	memory_write(address_hi_byte, hi_byte);

	// Move the stack pointer to the new empty address
	this->stack_pointer -= 2;
}


uint8_t CPU::pop_stack() {
	if (this->stack_pointer == 0xFF) {
		// Wrap the stack pointer if underflow occurs
		this->stack_pointer = 0x00;
	}
	// Get the top item of the stack
	uint16_t address = 0x0100 + this->stack_pointer + 1;

	uint8_t data = memory_read(address);
	memory_write(address, 0); // Clear the memory
	this->stack_pointer = address;
	
	return data;
}


uint16_t CPU::pop_stack_uint16() {
	// Wrap the stack pointer if underflow occurs
	if (this->stack_pointer == 0xFF)  {
		this->stack_pointer = 0x01;
	} else if (this->stack_pointer == 0xFE) {
		this->stack_pointer = 0x00;
	}
	uint16_t address_hi_byte = 0x0100 + this->stack_pointer + 1;
	uint16_t address_lo_byte = 0x0100 + this->stack_pointer + 2;

	uint16_t hi_byte = memory_read(address_hi_byte) << 8;
	uint16_t lo_byte = memory_read(address_lo_byte);
	// Clear the memory
	memory_write(address_hi_byte, 0);
	memory_write(address_lo_byte, 0);

	this->stack_pointer = (uint8_t)(address_lo_byte - 0x0100);

	return hi_byte | lo_byte;
}


void CPU::load_program(const std::vector<uint8_t> program) {
	// The memory space of the ROM on the NES is from 0x8000 to 0xFFFF
	// Throw an error if the program does not fit into memory
	if (program.size() > UINT16_MAX/2) {
		throw std::out_of_range("Program does not fit into memory...");
	}
	if (program.size() == 0) {
		throw std::out_of_range("Program does not contain any instructions...");
	}

	// const uint16_t program_length = program.size(); 
	// for (int i = 0; i <= program_length; i++) {
	// 	this->memory[0x8000+i] = program[i]; // load the program into memory
	// }
	// this->memory_write_uint16(0xFFFC, 0x8000);

	// Updated for snake game
	const uint16_t program_length = program.size(); 
	for (int i = 0; i <= program_length; i++) {
		this->memory[0x0600+i] = program[i]; // load the program into memory
	}
	// Write location of the first byte
	this->memory_write_uint16(0xFFFC, 0x0600);
}


void CPU::load_program_and_run(const std::vector<uint8_t> program) {
	this->load_program(program);
	this->reset();
	this->run();
}


void CPU::reset() {
	this->register_a = 0;
	this->register_irx = 0;
	this->register_iry = 0;
	this->status = 0;
	this->cycles = 0;

	uint16_t first_instruction_address = 0xFFFC;
	this->program_counter = memory_read_uint16(first_instruction_address);
}


void CPU::reset_memory_space() {
	for (int i = 0; i < 0xFFFF; i++) {
		this->memory[i] = 0;
	}
}


void CPU::execute_instruction(const uint8_t opcode) {
	// [TODO]: Wrap the instruction in an enum for better matching
	// Move updating the program counter to the get_operand_address function?
	switch(opcode) {
		case 0x69: {
			ADC(AddressingMode::Immediate);
			this->program_counter += 1;
			break;
		}
		case 0x65: {
			ADC(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 3;
			break;
		}
		case 0x75: {
			ADC(AddressingMode::ZeroPageX);
			this->program_counter += 1;
			this->cycles += 4;
			break;
		}
		case 0x6D: {
			ADC(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 4;
			break;
		}
		case 0x7D: {
			ADC(AddressingMode::AbsoluteX);
			this->cycles += 4;
			break;
		}
		case 0x79: {
			ADC(AddressingMode::AbsoluteY);
			this->program_counter += 2;
			this->cycles += 4;
			break;
		}
		case 0x61: {
			ADC(AddressingMode::IndirectX);
			this->program_counter += 1;
			this->cycles += 6;
			break;
		}
		case 0x71: {
			ADC(AddressingMode::IndirectY);
			this->program_counter += 1;
			this->cycles += 5;
			break;
		}
		case 0x29: {
			AND(AddressingMode::Immediate);
			this->program_counter += 1;
			this->cycles += 2;
			break;
		}
		case 0x25: {
			AND(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 3;
			break;
		}
		case 0x35: {
			AND(AddressingMode::ZeroPageX);
			this->program_counter += 1;
			this->cycles += 4;
			break;
		}
		case 0x2D: {
			AND(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 4;
			break;
		}
		case 0x3D: {
			AND(AddressingMode::AbsoluteX);
			this->program_counter += 2;
			this->cycles += 4;
			break;
		}
		case 0x39: {
			AND(AddressingMode::AbsoluteY);
			this->program_counter += 2;
			this->cycles += 4;
			break;
		}
		case 0x21: {
			AND(AddressingMode::IndirectX);
			this->program_counter += 1;
			this->cycles += 6;
			break;
		}
		case 0x31: {
			AND(AddressingMode::IndirectY);
			this->program_counter += 1;
			this->cycles += 5;
			break;
		}
		// ASL
		case 0x0A: {
			ASL(AddressingMode::Accumulator);
			this->cycles += 2;
			break;
		}
		case 0x06: {
			ASL(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 5;
			break;
		}
		case 0x16: {
			ASL(AddressingMode::ZeroPageX);
			this->program_counter += 1;
			this->cycles += 6;
			break;
		}
		case 0x0E: {
			ASL(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 6;
			break;
		}
		case 0x1E: {
			ASL(AddressingMode::AbsoluteX);
			this->program_counter += 2;
			this->cycles += 7;
			break;
		}
		case 0x90: {
			BCC();
			// Cycles should be +1 if the branch succeeds
			// and +2 if it is to a new page
			this->cycles += 2;
			break;
		}
		case 0xB0: {
			BCS();
			// Cycles should be +1 if the branch succeeds
			// and +2 if it is to a new page
			this->cycles += 2;
			break;
		}
		case 0xF0: {
			BEQ();
			// Cycles should be +1 if the branch succeeds
			// and +2 if it is to a new page
			this->cycles += 2;
			break;
		}
		case 0x24: {
			BIT(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 3;
			break;
		}
		case 0x2C: {
			BIT(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 4;
			break;
		}
		case 0x30: {
			BIT(AddressingMode::Relative);
			// Cycles should be +1 if the branch succeeds
			// and +2 if it is to a new page
			this->cycles += 2;
			break;
		}
		case 0xD0: {
			BNE();
			// Cycles should be +1 if the branch succeeds
			// and +2 if it is to a new page
			this->cycles += 2;
			break;
		}
		case 0x10: {
			BPL();
			// Cycles should be +1 if the branch succeeds
			// and +2 if it is to a new page
			this->cycles += 2;
			break;
		}
		case 0x00: {
			BRK();
			this->cycles += 7;
			return;
			// break;
		}
		case 0x50: {
			BRK();
			// Cycles should be +1 if the branch succeeds
			// and +2 if it is to a new page
			this->cycles += 2;
			break;
		}
		case 0x70: {
			BVS();
			// Cycles should be +1 if the branch succeeds
			// and +2 if it is to a new page
			this->cycles += 2;
			break;
		}
		case 0x18: {
			CLC();
			this->cycles += 2;
			break;
		}
		case 0xD8: {
			CLD();
			this->cycles += 2;
			break;
		}
		case 0x58: {
			CLI();
			this->cycles += 2;
			break;
		}
		case 0xB8: {
			CLV();
			this->cycles += 2;
			break;
		}
		case 0xC9: {
			CMP(AddressingMode::Immediate);
			this->program_counter += 1;
			this->cycles += 2;
			break;
		}
		case 0xC5: {
			CMP(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 3;
			break;
		}
		case 0xD5: {
			CMP(AddressingMode::ZeroPageX);
			this->program_counter += 1;
			this->cycles += 4;
			break;
		}
		case 0xCD: {
			CMP(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 4;
			break;
		}
		case 0xDD: {
			CMP(AddressingMode::AbsoluteX);
			this->program_counter += 2;
			// +1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0xD9: {
			CMP(AddressingMode::AbsoluteY);
			this->program_counter += 2;
			// +1 if page crossed
			this->cycles += 2;
			break;
		}
		case 0xC1: {
			CMP(AddressingMode::IndirectX);
			this->program_counter += 1;
			this->cycles += 6;
			break;
		}
		case 0xD1: {
			CMP(AddressingMode::IndirectY);
			this->program_counter += 1;
			// +1 if page crossed
			this->cycles += 2;
			break;
		}
		case 0xE0: {
			CPX(AddressingMode::Immediate);
			this->program_counter += 1;
			this->cycles += 2;
			break;
		}
		case 0xE4: {
			CPX(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 3;
			break;
		}
		case 0xEC: {
			CPX(AddressingMode::AbsoluteX);
			this->program_counter += 2;
			this->cycles += 4;
			break;
		}
		case 0xC0: {
			CPY(AddressingMode::Immediate);
			this->program_counter += 1;
			this->cycles += 2;
			break;
		}
		case 0xC4: {
			CPY(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 3;
			break;
		}
		case 0xCC: {
			CPY(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 4;
			break;
		}
		case 0xC6: {
			DEC(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 5;
			break;
		}
		case 0xD6: {
			DEC(AddressingMode::ZeroPageX);
			this->program_counter += 1;
			this->cycles += 6;
			break;
		}
		case 0xCE: {
			DEC(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 6;
			break;
		}
		case 0xDE: {
			DEC(AddressingMode::AbsoluteX);
			this->program_counter += 2;
			this->cycles += 7;
			break;
		}
		case 0xCA: {
			DEX();
			this->cycles += 2;
			break;
		}
		case 0x88: {
			DEY();
			this->cycles += 2;
			break;
		}
		case 0x49: {
			EOR(AddressingMode::Immediate);
			this->program_counter += 1;
			this->cycles += 2;
			break;
		}
		case 0x45: {
			EOR(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 3;
			break;
		}
		case 0x55: {
			EOR(AddressingMode::ZeroPageX);
			this->program_counter += 1;
			this->cycles += 4;
			break;
		}
		case 0x4D: {
			EOR(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 4;
			break;
		}
		case 0x5D: {
			EOR(AddressingMode::AbsoluteX);
			this->program_counter += 2;
			// +1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0x59: {
			EOR(AddressingMode::AbsoluteY);
			this->program_counter += 2;
			// +1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0x41: {
			EOR(AddressingMode::IndirectX);
			this->program_counter += 1;
			this->cycles += 6;
			break;
		}
		case 0x51: {
			EOR(AddressingMode::IndirectY);
			this->program_counter += 1;
			// +1 if page crossed
			this->cycles += 5;
			break;
		}
		case 0xE6: {
			INC(AddressingMode::ZeroPage);
			this->program_counter += 1;
			// +1 if page crossed
			this->cycles += 5;
			break;
		}
		case 0xF6: {
			INC(AddressingMode::ZeroPageX);
			this->program_counter += 1;
			this->cycles += 6;
			break;
		}
		case 0xEE: {
			INC(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 6;
			break;
		}
		case 0xFE: {
			INC(AddressingMode::AbsoluteX);
			this->program_counter += 2;
			this->cycles += 7;
			break;
		}
		case 0xE8: {
			INX();
			this->cycles += 2;
			break;
		}
		case 0xC8: {
			INY();
			this->cycles += 2;
			break;
		}
		case 0x4C: {
			JMP(AddressingMode::Absolute);
			// Modifies the program counter, no increment
			this->cycles += 3;
			break;
		}
		case 0x6C: {
			JMP(AddressingMode::Indirect);
			this->cycles += 5;
			break;
		}
		case 0x20: {
			JSR();
			this->cycles += 6;
			break;
		}
		case 0xA9: {
			LDA(AddressingMode::Immediate);
			this->program_counter += 1;
			this->cycles += 2;
			break;
		}
		case 0xA5: {
			LDA(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 3;
			break;
		}
		case 0xB5: {
			LDA(AddressingMode::ZeroPageX);
			this->program_counter += 1;
			this->cycles += 4;
			break;
		}
		case 0xAD: {
			LDA(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 4;
			break;
		}
		case 0xBD: {
			LDA(AddressingMode::AbsoluteX);
			this->program_counter += 2;
			// +1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0xB9: {
			LDA(AddressingMode::AbsoluteY);
			this->program_counter += 2;
			// +1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0xA1: {
			LDA(AddressingMode::IndirectX);
			this->program_counter += 1;
			this->cycles += 6;
			break;
		}
		case 0xB1: {
			LDA(AddressingMode::IndirectY);
			this->program_counter += 1;
			// +1 if page crossed
			this->cycles += 5;
			break;
		}
		case 0xA2: {
			LDX(AddressingMode::Immediate);
			this->program_counter += 1;
			this->cycles += 2;
			break;
		}
		case 0xA6: {
			LDX(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 3;
			break;
		}
		case 0xB6: {
			LDX(AddressingMode::ZeroPageY);
			this->program_counter += 1;
			this->cycles += 4;
			break;
		}
		case 0xAE: {
			LDX(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 4;
			break;
		}
		case 0xBE: {
			LDX(AddressingMode::AbsoluteY);
			this->program_counter += 2;
			// +1 if page crossed
			this->cycles += 4;
			break;
		}

		case 0xA0: {
			LDY(AddressingMode::Immediate);
			this->program_counter += 1;
			this->cycles += 2;
			break;
		}
		case 0xA4: {
			LDY(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 4;
			break;
		}
		case 0xB4: {
			LDY(AddressingMode::ZeroPageX);
			this->program_counter += 1;
			this->cycles += 4;
			break;
		}
		case 0xAC: {
			LDY(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 4;
			break;
		}
		case 0xBC: {
			LDY(AddressingMode::AbsoluteX);
			this->program_counter += 2;
			// +1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0x4A: {
			LSR(AddressingMode::Accumulator);
			this->cycles += 2;
			break;
		}
		case 0x46: {
			LSR(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 5;
			break;
		}
		case 0x56: {
			LSR(AddressingMode::ZeroPageX);
			this->program_counter += 1;
			this->cycles += 6;
			break;
		}
		case 0x4E: {
			LSR(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 6;
			break;
		}
		case 0x5E: {
			LSR(AddressingMode::AbsoluteX);
			this->program_counter += 2;
			this->cycles += 7;
			break;
		}
		case 0xEA: {
			NOP();
			this->cycles += 2;
			break;
		}
		case 0x09: {
			ORA(AddressingMode::Immediate);
			this->program_counter += 1;
			this->cycles += 2;
			break;
		}
		case 0x05: {
			ORA(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 3;
			break;
		}
		case 0x15: {
			ORA(AddressingMode::ZeroPageX);
			this->program_counter += 1;
			this->cycles += 4;
			break;
		}
		case 0x0D: {
			ORA(AddressingMode::Absolute);
			this->program_counter += 2; // Look for D0
			this->cycles += 4;
			break;
		}
		case 0x1D: {
			ORA(AddressingMode::AbsoluteX);
			this->program_counter += 2;
			// +1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0x19: {
			ORA(AddressingMode::AbsoluteY);
			this->program_counter += 2;
			// +1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0x01: {
			ORA(AddressingMode::IndirectX);
			this->program_counter += 1;
			this->cycles += 6;
			break;
		}
		case 0x11: {
			ORA(AddressingMode::IndirectY);
			this->program_counter += 1;
			// +1 if page crossed
			this->cycles += 5;
			break;
		}
		case 0x48: {
			PHA();
			this->cycles += 3;
			break;
		}
		case 0x08: {
			PHP();
			this->cycles += 3;
			break;
		}
		case 0x68: {
			PLA();
			this->cycles += 3;
			break;
		}
		case 0x28: {
			PLP();
			this->cycles += 4;
			break;
		}
		case 0x2A: {
			ROL(AddressingMode::Accumulator);
			this->cycles += 2;
			break;
		}
		case 0x26: {
			ROL(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 5;
			break;
		}
		case 0x36: {
			ROL(AddressingMode::ZeroPageX);
			this->program_counter += 1;
			this->cycles += 6;
			break;
		}
		case 0x2E: {
			ROL(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 6;
			break;
		}
		case 0x3E: {
			ROL(AddressingMode::AbsoluteX);
			this->program_counter += 2;
			this->cycles += 7;
			break;
		}
		case 0x6A: {
			ROR(AddressingMode::Accumulator);
			this->cycles += 2;
			break;
		}
		case 0x66: {
			ROR(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 5;
			break;
		}
		case 0x76: {
			ROR(AddressingMode::ZeroPageX);
			this->program_counter += 1;
			this->cycles += 6;
			break;
		}
		case 0x6E: {
			ROR(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 6;
			break;
		}
		case 0x7E: {
			ROR(AddressingMode::AbsoluteX);
			this->program_counter += 2;
			this->cycles += 7;
			break;
		}
		case 0x40: {
			RTI();
			this->cycles += 6;
			break;
		}
		case 0x60: {
			RTS();
			this->cycles += 6;
			break;
		}
		case 0xE9: {
			SBC(AddressingMode::Immediate);
			this->program_counter += 1;
			this->cycles += 2;
			break;
		}
		case 0xE5: {
			SBC(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 3;
			break;
		}
		case 0xF5: {
			SBC(AddressingMode::ZeroPageX);
			this->program_counter += 1;
			this->cycles += 4;
			break;
		}
		case 0xED: {
			SBC(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 4;
			break;
		}
		case 0xFD: {
			SBC(AddressingMode::AbsoluteX);
			this->program_counter += 2;
			// + 1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0xF9: {
			SBC(AddressingMode::AbsoluteY);
			this->program_counter += 2;
			// + 1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0xE1: {
			SBC(AddressingMode::IndirectX);
			this->program_counter += 1;
			this->cycles += 6;
			break;
		}
		case 0xF1: {
			SBC(AddressingMode::IndirectY);
			this->program_counter += 1;
			this->cycles += 5;
			break;
		}
		case 0x38: {
			SEC();
			this->cycles += 2;
			break;
		}
		case 0xF8: {
			SED();
			this->cycles += 2;
			break;
		}
		case 0x78: {
			SEI();
			this->cycles += 2;
			break;
		}
		case 0x85: {
			STA(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 3;
			break;
		}
		case 0x95: {
			STA(AddressingMode::ZeroPageX);
			this->program_counter += 1;
			this->cycles += 4;
			break;
		}
		case 0x8D: {
			STA(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 4;
			break;
		}
		case 0x9D: {
			STA(AddressingMode::AbsoluteX);
			this->program_counter += 2;
			this->cycles += 5;
			break;
		}
		case 0x99: {
			STA(AddressingMode::AbsoluteY);
			this->program_counter += 2;
			this->cycles += 5;
			break;
		}
		case 0x81: {
			STA(AddressingMode::IndirectX);
			this->program_counter += 1;
			this->cycles += 6;
			break;
		}
		case 0x91: {
			STA(AddressingMode::IndirectY);
			this->program_counter += 1;
			this->cycles += 6;
			break;
		}
		case 0x86: {
			STX(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 3;
			break;
		}
		case 0x96: {
			STX(AddressingMode::ZeroPageY);
			this->program_counter += 1;
			this->cycles += 4;
			break;
		}
		case 0x8E: {
			STX(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 4;
			break;
		}
		case 0x84: {
			STY(AddressingMode::ZeroPage);
			this->program_counter += 1;
			this->cycles += 3;
			break;
		}
		case 0x94: {
			STY(AddressingMode::ZeroPageX);
			this->program_counter += 1;
			this->cycles += 4;
			break;
		}
		case 0x8C: {
			STY(AddressingMode::Absolute);
			this->program_counter += 2;
			this->cycles += 4;
			break;
		}
		case 0xAA: {
			TAX();
			this->cycles += 2;
			break;
		}
		case 0xA8: {
			TAY();
			this->cycles += 2;
			break;
		}
		case 0xBA: {
			TSX();
			this->cycles += 2;
			break;
		}
		case 0x8A: {
			TXA();
			this->cycles += 2;
			break;
		}
		case 0x9A: {
			TXS();
			this->cycles += 2;
			break;
		}
		case 0x98: {
			TYA();
			this->cycles += 2;
			break;
		}
		default: {
			//[TODO]: Make this error handling more elegant with custom exception type
			int opc = opcode;
			std::cerr << "Invalid OPCODE: " 
				<< std::setfill('0') << std::setw(2) 
				<< std::uppercase << std::hex 
				<< opc << std::dec << std::endl;
			break;
		}
	}
}


int CPU::interpret(std::vector<uint8_t> program) {
	while (this->program_counter < program.size()) {
		const uint8_t opcode = program[this->program_counter]; // Fetch the instruction
		this->program_counter += 1;
		execute_instruction(opcode);
	}
	return 0;
}


void CPU::run() {
	while (true) {
		uint8_t opcode = memory_read(this->program_counter);
		uint16_t pc = this->program_counter;
		uint32_t starting_cycles = this->cycles;

		if (opcode == 0x00) {
			this->log_instruction(pc, OPCODES[0x00]);
			break; // Exit if opcode is 0x00
		}
		this->program_counter += 1;
		this->execute_instruction(opcode);

		// Debug info
		Opcode opcode_data = OPCODES[opcode];
		this->log_instruction(pc, opcode_data);

		this->wait_cycle_count(this->cycles - starting_cycles);
	}
}


void CPU::run_callback(const std::function<void(CPU*)>& callback) {
	while (true) {
		uint8_t opcode = memory_read(this->program_counter);
		uint16_t pc = this->program_counter;
		uint32_t starting_cycles = this->cycles;

		if (opcode == 0x00) {
			break; // Exit if opcode is 0x00
		}
		this->program_counter += 1;

		// Execute the callback to check for user input
		callback(this);
		this->execute_instruction(opcode);

		this->wait_cycle_count(this->cycles - starting_cycles);
		std::this_thread::sleep_for(std::chrono::nanoseconds(700000));
	}
}

void CPU::wait_cycle_count(uint8_t cycles) {
	for (int i = 0; i < cycles; i++) {
		std::this_thread::sleep_for(std::chrono::nanoseconds(this->cycle_duration));
	}
}


void CPU::NOP() { } // Does literally nothing, adds a cycle to the cycle counter?


void CPU::ADC(const AddressingMode mode) {
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);
	this->fetched_data = operand;

	add_to_accumulator_register(operand);

	update_zero_and_negative_flags(this->register_a);
}


void CPU::BCC() {
	if ((this->status & Flag::Carry) == 0) {
		this->program_counter = this->branch();
	} else {
		// Increment the program counter in case the branch fails.
		this->program_counter += 1;
	}
}


void CPU::BCS() {
	if ((this->status & Flag::Carry) == Flag::Carry) {
		this->program_counter = branch();
	} else {
		// Increment the program counter in case the branch fails.
		this->program_counter += 1;
	}
}


void CPU::BEQ() { 
	if ((this->status & Flag::Zero) == Flag::Zero) {
		this->program_counter = branch();
	} else {
		// Increment the program counter in case the branch fails.
		this->program_counter += 1;
	}
}


void CPU::BIT(const AddressingMode mode) {
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);
	this->fetched_data = operand;
	const uint8_t bitmask = this->register_a;

	// Take the logical AND 
	const uint8_t result = (operand & bitmask);

	// Update N and Z flags
	update_zero_and_negative_flags(result);
	
	// Update V flag
	if ((result & Flag::Overflow) != 0) {
		update_flag(Flag::Overflow, Mode::Set);
	} else {
		update_flag(Flag::Overflow, Mode::Clear);
	}
}


void CPU::BMI() {
	if ((this->status & Flag::Negative) == Flag::Negative) {
		this->program_counter = this->branch();
	} else {
		// Increment the program counter in case the branch fails.
		this->program_counter += 1;
	}
}


void CPU::BNE() {
	if ((this->status & Flag::Zero) == 0) {
		// Branch if the Z flag is NOT set
		this->program_counter = this->branch();
	} else {
		// Increment the program counter in case the branch fails.
		this->program_counter += 1;
	}
}


void CPU::BPL() {
	if ((this->status & Flag::Zero) == 0) { 
		this->program_counter = this->branch();
	} else {
		// Increment the program counter in case the branch fails.
		this->program_counter += 1;
	}
}


void CPU::BVC() {
	if ((this->status & Flag::Overflow) == 0) { 
		this->program_counter = this->branch();
	} else {
		// Increment the program counter in case the branch fails.
		this->program_counter += 1;
	}
}


void CPU::BVS() {
	if ((this->status & Flag::Overflow) != 0) { 
		this->program_counter = this->branch();
	} else {
		// Increment the program counter in case the branch fails.
		this->program_counter += 1;
	}
}


void CPU::CLC() {
	update_flag(Flag::Carry, Mode::Clear);
}


void CPU::CLD() {
	update_flag(Flag::DecimalMode, Mode::Clear);
}


void CPU::CLI() {
	update_flag(Flag::InteruptDisable, Mode::Clear);
}


void CPU::CLV() {
	update_flag(Flag::Overflow, Mode::Clear);
}


// Look into getting this to work
void CPU::BRK() {
	// // Push program counter and processor status onto the stack
	// uint16_t interrupt_vector = 0xFFFE;
	// this->program_counter = memory_read(interrupt_vector);
	// // Set the break flag to 1
	// this->status = this->status | Flag::Break;
}


void CPU::CMP(const AddressingMode mode) {
	this->compare(this->register_a, mode);
}


void CPU::CPX(const AddressingMode mode) {
	this->compare(this->register_irx, mode);
}


void CPU::CPY(const AddressingMode mode) {
	this->compare(this->register_iry, mode);
}


void CPU::DEC(const AddressingMode mode) {
	const uint16_t operand_addres = get_operand_address(mode);
	const uint8_t value = memory_read(operand_addres);
	this->fetched_data = value;

	memory_write(operand_addres, value-1);
	update_zero_and_negative_flags(value-1);
}


void CPU::DEX() {
	this->register_irx -= 1;
	update_zero_and_negative_flags(this->register_irx);
}


void CPU::DEY() {
	this->register_iry -= 1;
	update_zero_and_negative_flags(this->register_iry);
}


void CPU::EOR(const AddressingMode mode) {
	const uint16_t operand_addres = get_operand_address(mode);
	const uint8_t value = memory_read(operand_addres);
	this->fetched_data = value;

	this->register_a = this->register_a ^ value;
	update_zero_and_negative_flags(this->register_a);
}


void CPU::INC(const AddressingMode mode) {
	const uint16_t operand_addres = get_operand_address(mode);
	const uint8_t value = memory_read(operand_addres);
	this->fetched_data = value;

	memory_write(operand_addres, value+1);
	update_zero_and_negative_flags(value+1);
}


void CPU::INX() {
	this->register_irx += 1;
	update_zero_and_negative_flags(this->register_irx);
}


void CPU::INY() {
	this->register_iry += 1;
	update_zero_and_negative_flags(this->register_iry);
}


void CPU::JMP(const AddressingMode mode) {
	const uint16_t address = get_operand_address(mode);
	this->fetched_data = address;
	this->program_counter = address;
}


void CPU::JSR() {
	// Push current program_counter - 1 to the stack as return address.
	const uint16_t return_address = this->program_counter + 1;
	push_stack_uint16(return_address);

	// Get the subroutine address and set the program counter to this address
	const uint16_t address = get_operand_address(AddressingMode::Absolute);
	this->fetched_data = address;
	this->program_counter = address;
} 


void CPU::LDA(const AddressingMode mode) {
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);
	this->fetched_data = operand;

	this->register_a = operand;
	update_zero_and_negative_flags(this->register_a);
}


void CPU::LDX(const AddressingMode mode) {
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);
	this->fetched_data = operand;

	this->register_irx = operand;
	update_zero_and_negative_flags(this->register_irx);
}


void CPU::LDY(const AddressingMode mode) {
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);
	this->fetched_data = operand;

	this->register_iry = operand;
	update_zero_and_negative_flags(this->register_iry);
}


uint8_t CPU::LSR(const AddressingMode mode) {
	// Why is it Logical Shift Right but Arithmatic Shift Left???
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);
	this->fetched_data = operand;

	// Set the carry flag if the first bit is set
	if ((operand & 0b00000001) == 0) {
		update_flag(Flag::Carry, Mode::Clear);
	} else {
		update_flag(Flag::Carry, Mode::Set);
	}

	const uint8_t result = operand >> 1;

	if (mode == AddressingMode::Accumulator) {
		this->register_a = result;
	} else {
		memory_write(operand_address, result);
	}
	update_zero_and_negative_flags(result);

	return result;
}


void CPU::ORA(const AddressingMode mode) {
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);
	this->fetched_data = operand;

	this->register_a = this->register_a | operand;
	update_zero_and_negative_flags(this->register_a);
}


void CPU::PHA() {
	push_stack(this->register_a);
}


void CPU::PHP() {
	push_stack(this->status);
}


void CPU::PLA() {
	this->register_a = pop_stack();
}


void CPU::PLP() {
	this->status = pop_stack();
}


void CPU::ROL(const AddressingMode mode) {
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);
	this->fetched_data = operand;

	uint8_t result = operand << 1;

	// Update the 0 bit by using the old Carry flag
	if ((this->status & Flag::Carry) == Flag::Carry) {
		// Flag is set	
		result = result | 0x01;
	} 

	// Update the Carry flag to the new value
	if ((operand & 0x80) == 0) {
		// bit 7 not set
		update_flag(Flag::Carry, Mode::Clear);
	} else {
		update_flag(Flag::Carry, Mode::Set);
	}

	update_zero_and_negative_flags(result);

	// Write the value to the correct location (either accumulator or into memory
	if (mode == AddressingMode::Accumulator) {
		this->register_a = result;
	} else {
		memory_write(operand_address, result);
	}

}


void CPU::ROR(const AddressingMode mode) {
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);
	this->fetched_data = operand;

	uint8_t result = operand >> 1;

	// Update the 0 bit by using the old Carry flag
	if ((this->status & Flag::Carry) == Flag::Carry) {
		// Flag is set	
		result = result | 0b10000000;
	} 

	// Update the Carry flag to the new value
	if ((operand & 0b00000001) == 0) {
		// bit 7 not set
		update_flag(Flag::Carry, Mode::Clear);
	} else {
		update_flag(Flag::Carry, Mode::Set);
	}

	update_zero_and_negative_flags(result);

	// Write the value to the correct location (either accumulator or into memory
	if (mode == AddressingMode::Accumulator) {
		this->register_a = result;
	} else {
		memory_write(operand_address, result);
	}
}


void CPU::RTI() {
	this->status = pop_stack();
	this->program_counter = pop_stack_uint16();
}


void CPU::RTS() {
	this->program_counter = pop_stack_uint16() + 1;
}


void CPU::SBC(const AddressingMode mode) {
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);
	this->fetched_data = operand;
	subtract_from_accumulator_register(operand);

	update_zero_and_negative_flags(this->register_a);
}


void CPU::SEC() {
	update_flag(Flag::Carry, Mode::Set);
}


void CPU::SED() {
	update_flag(Flag::DecimalMode, Mode::Set);
}


void CPU::SEI() {
	update_flag(Flag::InteruptDisable, Mode::Set);
}


void CPU::STA(const AddressingMode mode) {
	const uint16_t address = get_operand_address(mode);
	this->fetched_data = address;
	memory_write(address, this->register_a);
}


void CPU::STX(const AddressingMode mode) {
	const uint16_t address = get_operand_address(mode);
	this->fetched_data = address;
	memory_write(address, this->register_irx);
}


void CPU::STY(const AddressingMode mode) {
	const uint16_t address = get_operand_address(mode);
	this->fetched_data = address;
	memory_write(address, this->register_iry);
}


void CPU::TAX() {
	this->register_irx = this->register_a;
	update_zero_and_negative_flags(this->register_irx);
}


void CPU::TAY() {
	this->register_iry = this->register_a;
	update_zero_and_negative_flags(this->register_iry);
}


void CPU::TSX() {
	this->register_irx = this->stack_pointer;
	update_zero_and_negative_flags(this->register_irx);
}


void CPU::TXA() {
	this->register_a = this->register_irx;
	update_zero_and_negative_flags(this->register_a);
}


void CPU::TXS() {
	this->stack_pointer = this->register_irx;
}


void CPU::TYA() {
	this->register_a = this->register_iry;
}


void CPU::AND(const AddressingMode mode) {
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);
	this->fetched_data = operand;

	// Update register and flags
	this->register_a = this->register_a & operand;
	update_zero_and_negative_flags(this->register_a);
}


uint8_t CPU::ASL(const AddressingMode mode) {
	uint16_t operand_adress = get_operand_address(mode);
	uint8_t operand = memory_read(operand_adress);
	this->fetched_data = operand;

	if ((operand & 0b10000000) == 0) { 
		// If this is not 0, then 1 should be added to the carry
		update_flag(Flag::Carry, Mode::Clear);
	} else {
		update_flag(Flag::Carry, Mode::Set);
	}

	uint8_t result = operand << 1;

	if (mode == AddressingMode::Accumulator) {
		// store in the accumulator if addressing mode is Accumulator
		this->register_a = result;
	} else {
		memory_write(operand_adress, result);
	}

	update_zero_and_negative_flags(result);
	// return the result (also return if it is in the accumulator)
	return result;
}


uint16_t CPU::branch() {
	int8_t jmp = memory_read(this->program_counter);
	uint16_t jmp_addr = this->program_counter + jmp + 1; 

	// 1 extra cycle in case as the branch was succesfull.
	this->cycles += 1;
	this->fetched_data = jmp;


	// if ((jmp & 0x80) != 0) {
	// 	// Sign bit is set, jump should be backwards, use 2's complement
	// 	// + 1 to correct for program counter being updated to read the operand
	// 	uint8_t jmp_comp = (uint8_t)(~jmp + 1);
	// 	jmp_addr = this->program_counter - jmp_comp + 1;
	// } else {
	// 	jmp_addr = this->program_counter + jmp + 1; 
	// }

	// Update the program counter
	return jmp_addr;
}


void CPU::compare(const uint8_t reg, const AddressingMode mode) {
	uint16_t operand_address = this->get_operand_address(mode);
	uint8_t operand = memory_read(operand_address);
	this->fetched_data = operand;

	if (operand == reg) {
		update_flag(Flag::Zero, Mode::Set);
	} else if (reg > operand) {
		update_flag(Flag::Carry, Mode::Set);
	} else if (((reg - operand) & 0b10000000) == 0) {
		update_flag(Flag::Negative, Mode::Set);
	}
}


void CPU::add_to_accumulator_register(const uint8_t operand) {
	uint16_t sum = (uint16_t)this->register_a + operand;

	if ((this->status & Flag::Carry) != 0) {
		// add the carry if the flag is set
		sum += 1;
		update_flag(Flag::Carry, Mode::Clear);
	}

	// Carry if sum is larger then what fits in the 8-bit register
	if (sum > 0xFF) {
		update_flag(Flag::Carry, Mode::Set);
	}

	const uint8_t result = (uint8_t)sum;

	// Check for overflow
	// Overflow occurs when adding 2 positive numbers results in a negative number
	// OR
	// adding 2 negative number results in a positive number.
	// This can be checked using
	//		result XOR operand, MSB = 1 if the result is of oppsite sign of the operand
	//		result XOR register_a, MSB = 1 if the result is of oppsite sign of the register content
	// If both of these are true simulataneously, then one of the following 2 situations happened
	//		Adding a positive number to the register with positive content resulted in a negative number
	//		Adding a negative number to the register with negative content resulted in a positive number
	// Implying that overflow has happened
	if ((result ^ operand) & (result ^ this->register_a) & 0x80) {
		update_flag(Flag::Overflow, Mode::Set);
	} else {
		update_flag(Flag::Overflow, Mode::Clear);
	}

	this->register_a = result;
}


void CPU::subtract_from_accumulator_register(const uint8_t operand) {
	uint16_t diff = (uint16_t)this->register_a - operand;

	if ((this->status & Flag::Carry) != 0) {
		// add the carry if the flag is set
		diff -= 1;
		update_flag(Flag::Carry, Mode::Clear);
	}

	// Carry if the result does not fit in an 8-bit register
	if (diff > 0x100) {
		update_flag(Flag::Carry, Mode::Set);
	}

	const uint8_t result = (uint8_t)diff;

	// Check for overflow:
	//	If subtracting a positive number from a negative number yielded a positive result
	//	OR
	//	If subtracting a negative number from a positive number yielded a negative result
	//	THEN
	//	the overflow bit should be set
	//	This can probably be rewritten into a more elegant expression using boolean algebra
	//	but for now this works and is fine.
	if (((this->register_a & 0x80) && !(operand & 0x80) && !(result & 0x80))
		|| (!(this->register_a & 0x80) && (operand & 0x80) && (result & 0x80))) {
		update_flag(Flag::Overflow, Mode::Set);
	} else {
		update_flag(Flag::Overflow, Mode::Clear);
	}

	this->register_a = result;
}


void CPU::update_flag(const Flag flag, const Mode mode) {
	if (mode == Mode::Set) {
		this->status = this->status | flag;
	} else if (mode == Mode::Clear) {
		this->status = this->status & ~flag;
	} else if (mode == Mode::Update) {
		// Check the current status of the register
		// if it's 1, unset it, otherwise set it
		const uint8_t register_status = this->status & flag;
		if (register_status == 0) {
			this->status = this->status | flag;
		}
		else if (register_status == 1) {
			this->status = this->status & ~flag;
		}
	}
}


void CPU::update_zero_and_negative_flags(const uint8_t reg) {
	if (reg == 0) { // Set the flag if the register content is 0
		this->status = this->status | Flag::Zero; // Set the Z flag
	} else {
		this->status = this->status & ~Flag::Zero; 
	}
	if ((reg & Flag::Negative) != 0) { // Set this flag if the negative bit is set
		this->status = this->status | Flag::Negative; // Set the N flag
	} else {
		this->status = this->status & ~Flag::Negative;
	}
}


uint16_t CPU::get_operand_address(const AddressingMode mode) {
	switch(mode) {
		case AddressingMode::Immediate: {
			return this->program_counter;
		}
		case AddressingMode::Relative: {
			uint16_t jmp_addr;

			// Get the offset
			uint8_t jmp = memory_read(this->program_counter);

			if ((jmp & 0x80) != 0) {
				// Sign bit set, offset negative
				// This can also be solved by taking the offset as a signed
				// 8 bit integer, however checking this way is more explicit
				jmp_addr = this->program_counter + 1 - jmp;
			} else {
				// Offset is positive, jump is forward in memory
				jmp_addr = this->program_counter + 1 + jmp;
			}
			return jmp_addr;
		}
		case AddressingMode::Accumulator: {
			return this->register_a;
		}
		case AddressingMode::ZeroPage: {
			return (uint16_t)memory_read(this->program_counter);
		}
		// C++ does wrapping addition by default on uint8 and uint16 types
		case AddressingMode::ZeroPageX: {
			const uint8_t pos = memory_read(this->program_counter);
			const uint16_t addr = (uint16_t)(pos + register_irx); 
			return addr;
		}
		case AddressingMode::ZeroPageY: {
			const uint8_t pos = memory_read(this->program_counter);
			const uint16_t addr = (uint16_t)(pos + register_iry);
			return addr;
		}
		case AddressingMode::Absolute: {
			uint16_t address = this->program_counter;
			return memory_read_uint16(address);
		}
		case AddressingMode::AbsoluteX: {
			uint16_t base = memory_read_uint16(this->program_counter);
			uint16_t addr = base + this->register_irx;
			return addr;
		}
		case AddressingMode::AbsoluteY: {
			uint16_t base = memory_read_uint16(this->program_counter);
			uint16_t addr = base + this->register_iry;
			return addr;
		}
		case AddressingMode::Indirect: {
			uint16_t ptr = memory_read_uint16(this->program_counter); // Read the address
			return memory_read_uint16(ptr); // read the data at the ptr location
		}
		case AddressingMode::IndirectX: {
			uint8_t base = memory_read(this->program_counter);
			uint8_t ptr = base + this->register_irx;

			uint16_t lo_byte = memory_read(ptr);
			uint16_t hi_byte = memory_read(ptr+1);

			return (hi_byte << 8) | lo_byte;
		}
		case AddressingMode::IndirectY: {
			uint8_t base = memory_read(this->program_counter);
			uint16_t lo_byte = memory_read(base);
			uint16_t hi_byte = memory_read(base+1);

			uint16_t deref_base = (hi_byte << 8) | lo_byte;
			uint16_t deref = deref_base + this->register_iry;

			if ((deref & 0xFF00) != (hi_byte << 8)) {
				this->cycles += 1;
			}

			return deref;
		}
		case AddressingMode::Implied: {
			// Here for completeness
			return 0;
		}
		default: { // Throw an error if the AddressingMode is not in the list
			throw std::runtime_error("Enum element not found");
		}
	}
	return 0;
}


void CPU::hex_dump(int lower_bound, int upper_bound) {
	const char* cdefault = "\033[0m";
	const char* cyellow = "\033[33m";

	// Round the lower bound down
	if ((lower_bound % 16) != 0) {
		lower_bound = lower_bound - (lower_bound%16);
	}
	// Round the upper bound up
	if ((upper_bound % 16) != 0) {
		upper_bound = upper_bound + (upper_bound%16);
	}

	for (int i = lower_bound; i < upper_bound; i += 16) {
		std::cout << cyellow << std::setfill('0') << std::setw(4) << std::uppercase << std::hex << i << ": " << cdefault;
		for (int j = i; j < i+16; j++) {
			int val = this->memory[j];
			// if (val != 0) {
			// 	std::cout << cblue;
			// } 
			std::cout << std::setfill('0') << std::setw(2) << std::hex << val << " " << cdefault;

			if (j-i == 7) {
				// Insert extra space in between bytes
				std::cout << " ";
			}
		}
		std::cout << std::dec << std::endl;
	}
}


void CPU::hex_dump() {
	int first_memory_address = 0x0000;
	int last_memory_address = 0xFFFF;
	this->hex_dump(first_memory_address, last_memory_address+1);
}


void CPU::hex_dump_zero_page() {
	// Stack lives in the memory range `0x0100` - `0x01FF`
	uint16_t zp_lb = 0x0000;
	uint16_t zp_ub = 0x00FF;
	this->hex_dump(zp_lb, zp_ub+1);
}


void CPU::hex_dump_stack() {
	// Stack lives in the memory range `0x0100` - `0x01FF`
	uint16_t stack_lower_bound = 0x0100;
	uint16_t stack_upper_bound = 0x01FF;
	this->hex_dump(stack_lower_bound, stack_upper_bound+1);
}


void CPU::hex_dump_rom() {
	// Hex dump the program ROM (lives at 0x8000 - 0xFFFF
	// However for the snake game this is instead set to the range 0x0600-0x0700
	uint16_t rom_lower_bound = 0x0600;
	uint16_t rom_upper_bound = 0x0740;
	this->hex_dump(rom_lower_bound, rom_upper_bound+1);
}


void CPU::log_instruction(const uint16_t pc, const Opcode opc) const {
	// Log the program counter
	std::cout << "$" << std::setw(4) << std::setfill('0') << std::hex << (int)pc << std::dec << ": " << opc.name;

	switch(opc.mode) {
		case AddressingMode::Accumulator: {
			std::cout << " A" << std::endl;
			break; 
		}
		case AddressingMode::Relative: {
			std::cout << " $" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
				<< (this->fetched_data & 0xFF) << std::dec << std::endl;
			break; 
		}
		case AddressingMode::Immediate: {
			std::cout << " #$" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
					 << this->fetched_data << std::dec << std::endl;
			break; 
		}
		case AddressingMode::ZeroPage: {
			std::cout << " $" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
					 << this->fetched_data << std::dec << std::endl;
			break; 
		}
		case AddressingMode::ZeroPageX: {
			std::cout << " $" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
					 << this->fetched_data << std::dec << ",X" << std::endl;
			break; 
		}
		case AddressingMode::ZeroPageY: {
			std::cout << " $" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
					 << this->fetched_data << std::dec << ",Y" << std::endl;
			break; 
		}
		case AddressingMode::IndirectX: {
			std::cout << " ($" << std::hex << std::uppercase << std::setw(4) << std::setfill('0')
					 << this->fetched_data << std::dec << ",X)" << std::endl;
			break; 
		}
		case AddressingMode::IndirectY: {
			std::cout << " ($" << std::hex << std::uppercase << std::setw(4) << std::setfill('0')
					 << this->fetched_data << std::dec << "),Y" << std::endl;
			break; 
		}
		case AddressingMode::Absolute: {
			std::cout << " $" << std::hex << std::uppercase << std::setw(4) << std::setfill('0')
					 << this->fetched_data << std::dec << std::endl;
			break; 
		}
		case AddressingMode::AbsoluteX: {
			std::cout << " $" << std::hex << std::uppercase << std::setw(4) << std::setfill('0')
					 << this->fetched_data << std::dec << ",X" << std::endl;
			break; 
		}
		case AddressingMode::AbsoluteY: {
			std::cout << " $" << std::hex << std::uppercase << std::setw(4) << std::setfill('0')
					 << this->fetched_data << std::dec << ",Y" << std::endl;
			break; 
		}
		case AddressingMode::Indirect: {
			std::cout << " $" << std::hex << std::uppercase << std::setw(4) << std::setfill('0')
					 << this->fetched_data << std::dec << std::endl;
			break; 
		}
		default: {
			std::cout << std::endl;
			break;
		}
	}
}


const std::bitset<8> as_binary8(const uint8_t val) {
	return std::bitset<8>(val);
}

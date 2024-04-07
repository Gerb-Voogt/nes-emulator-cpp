// [TODO]: Fix the order of the Opcdes in this file to be alphabetical
//	like the declaration order in 6502.hpp
#include <bitset>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <iomanip>

#include "mos6502.hpp"

CPU::CPU() {
	register_a = 0;
	register_irx = 0;
	register_iry = 0;
	program_counter = 0;
	stack_pointer = 0xFF;
	status = 0;
	cycles = 0;

	// Initialize memory space to 0
	for (int i = 0; i < 0xFFFF; i++) {
		memory[i] = 0;
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
	uint8_t hi_byte = (data << 8); // left shift by 8 to get the upper half of data into uint8_t
	uint8_t lo_byte = (data & 0b111111); // bitwise & with 255 in order to extract the lower half of data

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
	uint8_t hi_byte = (data << 8);
	uint8_t lo_byte = (data & 0b11111111);
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
	uint16_t address_hi_byte = 0x0100 + this->stack_pointer - 1;
	uint16_t address_lo_byte = 0x0100 + this->stack_pointer - 2;

	uint16_t hi_byte = memory_read(address_hi_byte) << 8;
	uint16_t lo_byte = memory_read(address_lo_byte);
	// Clear the memory
	memory_write(address_hi_byte, 0);
	memory_write(address_lo_byte, 0);

	this->stack_pointer = address_lo_byte;

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

	const uint16_t program_length = program.size(); 
	for (int i = 0; i <= program_length; i++) {
		this->memory[0x8000+i] = program[i]; // load the program into memory
	}
	this->memory_write_uint16(0xFFFC, 0x8000);
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

	uint16_t first_instruction = 0xFFFC;
	this->program_counter = memory_read_uint16(first_instruction);
}

void CPU::execute_instruction(const uint8_t opcode) {
	// [TODO]: Wrap the instruction in an enum for better matching
	switch (opcode) {
		case 0x69: {
			ADC(AddressingMode::Immediate);
			this->cycles += 2;
			break;
		}
		case 0x65: {
			ADC(AddressingMode::ZeroPage);
			this->cycles += 3;
			break;
		}
		case 0x75: {
			ADC(AddressingMode::ZeroPageX);
			this->cycles += 4;
			break;
		}
		case 0x6D: {
			ADC(AddressingMode::Absolute);
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
			this->cycles += 4;
			break;
		}
		case 0x61: {
			ADC(AddressingMode::IndirectX);
			this->cycles += 6;
			break;
		}
		case 0x71: {
			ADC(AddressingMode::IndirectY);
			this->cycles += 5;
			break;
		}
		case 0x29: {
			AND(AddressingMode::Immediate);
			this->cycles += 2;
			break;
		}
		case 0x25: {
			AND(AddressingMode::ZeroPage);
			this->cycles += 3;
			break;
		}
		case 0x35: {
			AND(AddressingMode::ZeroPageX);
			this->cycles += 4;
			break;
		}
		case 0x2D: {
			AND(AddressingMode::Absolute);
			this->cycles += 4;
			break;
		}
		case 0x3D: {
			AND(AddressingMode::AbsoluteX);
			this->cycles += 4;
			break;
		}
		case 0x39: {
			AND(AddressingMode::AbsoluteY);
			this->cycles += 4;
			break;
		}
		case 0x21: {
			AND(AddressingMode::IndirectX);
			this->cycles += 6;
			break;
		}
		case 0x31: {
			AND(AddressingMode::IndirectY);
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
			this->cycles += 5;
			break;
		}
		case 0x16: {
			ASL(AddressingMode::ZeroPageX);
			this->cycles += 6;
			break;
		}
		case 0x0E: {
			ASL(AddressingMode::Absolute);
			this->cycles += 6;
			break;
		}
		case 0x1E: {
			ASL(AddressingMode::AbsoluteX);
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
			this->cycles += 3;
			break;
		}
		case 0x2C: {
			BIT(AddressingMode::Absolute);
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
			break;
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
			this->cycles += 2;
			break;
		}
		case 0xC5: {
			CMP(AddressingMode::ZeroPage);
			this->cycles += 3;
			break;
		}
		case 0xD5: {
			CMP(AddressingMode::ZeroPageX);
			this->cycles += 4;
			break;
		}
		case 0xCD: {
			CMP(AddressingMode::Absolute);
			this->cycles += 4;
			break;
		}
		case 0xDD: {
			CMP(AddressingMode::AbsoluteX);
			// +1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0xD9: {
			CMP(AddressingMode::AbsoluteY);
			// +1 if page crossed
			this->cycles += 2;
			break;
		}
		case 0xC1: {
			CMP(AddressingMode::IndirectX);
			this->cycles += 6;
			break;
		}
		case 0xD1: {
			CMP(AddressingMode::IndirectY);
			// +1 if page crossed
			this->cycles += 2;
			break;
		}
		case 0xE0: {
			CPX(AddressingMode::Immediate);
			this->cycles += 2;
			break;
		}
		case 0xE4: {
			CPX(AddressingMode::ZeroPage);
			this->cycles += 3;
			break;
		}
		case 0xEC: {
			CPX(AddressingMode::AbsoluteX);
			this->cycles += 4;
			break;
		}
		case 0xC0: {
			CPY(AddressingMode::Immediate);
			this->cycles += 2;
			break;
		}
		case 0xC4: {
			CPY(AddressingMode::ZeroPage);
			this->cycles += 3;
			break;
		}
		case 0xCC: {
			CPY(AddressingMode::Absolute);
			this->cycles += 4;
			break;
		}
		case 0xC6: {
			DEC(AddressingMode::ZeroPage);
			this->cycles += 5;
			break;
		}
		case 0xD6: {
			DEC(AddressingMode::ZeroPageX);
			this->cycles += 6;
			break;
		}
		case 0xCE: {
			DEC(AddressingMode::Absolute);
			this->cycles += 6;
			break;
		}
		case 0xDE: {
			DEC(AddressingMode::AbsoluteX);
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
			this->cycles += 2;
			break;
		}
		case 0x45: {
			EOR(AddressingMode::ZeroPage);
			this->cycles += 3;
			break;
		}
		case 0x55: {
			EOR(AddressingMode::ZeroPageX);
			this->cycles += 4;
			break;
		}
		case 0x4D: {
			EOR(AddressingMode::Absolute);
			this->cycles += 4;
			break;
		}
		case 0x5D: {
			EOR(AddressingMode::AbsoluteX);
			// +1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0x59: {
			EOR(AddressingMode::AbsoluteY);
			// +1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0x41: {
			EOR(AddressingMode::IndirectX);
			this->cycles += 6;
			break;
		}
		case 0x51: {
			EOR(AddressingMode::IndirectY);
			// +1 if page crossed
			this->cycles += 5;
			break;
		}
		case 0xE6: {
			INC(AddressingMode::ZeroPage);
			// +1 if page crossed
			this->cycles += 5;
			break;
		}
		case 0xF6: {
			INC(AddressingMode::ZeroPageX);
			this->cycles += 6;
			break;
		}
		case 0xEE: {
			INC(AddressingMode::Absolute);
			this->cycles += 6;
			break;
		}
		case 0xFE: {
			INC(AddressingMode::AbsoluteX);
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
			this->cycles += 2;
			break;
		}
		case 0xA5: {
			LDA(AddressingMode::ZeroPage);
			this->cycles += 3;
			break;
		}
		case 0xB5: {
			LDA(AddressingMode::ZeroPageX);
			this->cycles += 4;
			break;
		}
		case 0xAD: {
			LDA(AddressingMode::Absolute);
			this->cycles += 4;
			break;
		}
		case 0xBD: {
			LDA(AddressingMode::AbsoluteX);
			// +1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0xB9: {
			LDA(AddressingMode::AbsoluteY);
			// +1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0xA1: {
			LDA(AddressingMode::IndirectX);
			this->cycles += 6;
			break;
		}
		case 0xB1: {
			LDA(AddressingMode::IndirectY);
			// +1 if page crossed
			this->cycles += 5;
			break;
		}
		case 0xA2: {
			LDX(AddressingMode::Immediate);
			this->cycles += 2;
			break;
		}
		case 0xA6: {
			LDX(AddressingMode::ZeroPage);
			this->cycles += 3;
			break;
		}
		case 0xB6: {
			LDX(AddressingMode::ZeroPageY);
			this->cycles += 4;
			break;
		}
		case 0xAE: {
			LDX(AddressingMode::Absolute);
			this->cycles += 4;
			break;
		}
		case 0xBE: {
			LDX(AddressingMode::AbsoluteY);
			// +1 if page crossed
			this->cycles += 4;
			break;
		}

		case 0xA0: {
			LDY(AddressingMode::Immediate);
			this->cycles += 2;
			break;
		}
		case 0xA4: {
			LDY(AddressingMode::ZeroPage);
			this->cycles += 4;
			break;
		}
		case 0xB4: {
			LDY(AddressingMode::ZeroPageX);
			this->cycles += 4;
			break;
		}
		case 0xAC: {
			LDY(AddressingMode::Absolute);
			this->cycles += 4;
			break;
		}
		case 0xBC: {
			LDY(AddressingMode::AbsoluteX);
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
			this->cycles += 5;
			break;
		}
		case 0x56: {
			LSR(AddressingMode::ZeroPageX);
			this->cycles += 6;
			break;
		}
		case 0x4E: {
			LSR(AddressingMode::Absolute);
			this->cycles += 6;
			break;
		}
		case 0x5E: {
			LSR(AddressingMode::AbsoluteX);
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
			this->cycles += 2;
			break;
		}
		case 0x05: {
			ORA(AddressingMode::ZeroPage);
			this->cycles += 3;
			break;
		}
		case 0x15: {
			ORA(AddressingMode::ZeroPageX);
			this->cycles += 4;
			break;
		}
		case 0x0D: {
			ORA(AddressingMode::Absolute);
			this->cycles += 4;
			break;
		}
		case 0x1D: {
			ORA(AddressingMode::AbsoluteX);
			// +1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0x19: {
			ORA(AddressingMode::AbsoluteY);
			// +1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0x01: {
			ORA(AddressingMode::IndirectX);
			this->cycles += 6;
			break;
		}
		case 0x11: {
			ORA(AddressingMode::IndirectY);
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
			this->cycles += 5;
			break;
		}
		case 0x36: {
			ROL(AddressingMode::ZeroPageX);
			this->cycles += 6;
			break;
		}
		case 0x2E: {
			ROL(AddressingMode::Absolute);
			this->cycles += 6;
			break;
		}
		case 0x3E: {
			ROL(AddressingMode::AbsoluteX);
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
			this->cycles += 5;
			break;
		}
		case 0x76: {
			ROR(AddressingMode::ZeroPageX);
			this->cycles += 6;
			break;
		}
		case 0x6E: {
			ROR(AddressingMode::Absolute);
			this->cycles += 6;
			break;
		}
		case 0x7E: {
			ROR(AddressingMode::AbsoluteX);
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
			this->cycles += 2;
			break;
		}
		case 0xE5: {
			SBC(AddressingMode::ZeroPage);
			this->cycles += 3;
			break;
		}
		case 0xF5: {
			SBC(AddressingMode::ZeroPageX);
			this->cycles += 4;
			break;
		}
		case 0xED: {
			SBC(AddressingMode::Absolute);
			this->cycles += 4;
			break;
		}
		case 0xFD: {
			SBC(AddressingMode::AbsoluteX);
			// + 1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0xF9: {
			SBC(AddressingMode::AbsoluteY);
			// + 1 if page crossed
			this->cycles += 4;
			break;
		}
		case 0xE1: {
			SBC(AddressingMode::IndirectX);
			this->cycles += 6;
			break;
		}
		case 0xF1: {
			SBC(AddressingMode::IndirectY);
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
			this->cycles += 3;
			break;
		}
		case 0x95: {
			STA(AddressingMode::ZeroPageX);
			this->cycles += 4;
			break;
		}
		case 0x8D: {
			STA(AddressingMode::Absolute);
			this->cycles += 4;
			break;
		}
		case 0x9D: {
			STA(AddressingMode::AbsoluteX);
			this->cycles += 5;
			break;
		}
		case 0x99: {
			STA(AddressingMode::AbsoluteY);
			this->cycles += 5;
			break;
		}
		case 0x81: {
			STA(AddressingMode::IndirectX);
			this->cycles += 6;
			break;
		}
		case 0x91: {
			STA(AddressingMode::IndirectY);
			this->cycles += 6;
			break;
		}
		case 0x86: {
			STX(AddressingMode::ZeroPage);
			this->cycles += 3;
			break;
		}
		case 0x96: {
			STX(AddressingMode::ZeroPageY);
			this->cycles += 4;
			break;
		}
		case 0x8E: {
			STX(AddressingMode::Absolute);
			this->cycles += 4;
			break;
		}
		case 0x84: {
			STY(AddressingMode::ZeroPage);
			this->cycles += 3;
			break;
		}
		case 0x94: {
			STY(AddressingMode::ZeroPageX);
			this->cycles += 4;
			break;
		}
		case 0x8C: {
			STY(AddressingMode::Absolute);
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
			std::cerr << "Invalid OPCODE: " << unsigned(opcode) << std::endl;
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
	// TODO: Fix this implementation. There might be something weird going on here with the program counter
	// counting down from `0xFFFF` rather than up from `0x8000`.
	while (true) {
		uint8_t opcode = memory_read(program_counter);
		this->program_counter += 1;
		this->execute_instruction(opcode);
	}
}

void CPU::NOP() { } // Does literally nothing, adds a cycle to the cycle counter?

void CPU::ADC(const AddressingMode mode) {
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);
	add_to_accumulator_register(operand);

	update_flag(Flag::Carry, Mode::Set);
	update_zero_and_negative_flags(this->register_a);
}

void CPU::BCC() {
	if ((this->status & Flag::Carry) == 0) {
		this->program_counter = this->branch();
	}
}

void CPU::BCS() {
	if ((this->status & Flag::Carry) == Flag::Carry) {
		this->program_counter = branch();
	}
}

void CPU::BEQ() { 
	if ((this->status & Flag::Zero) == Flag::Zero) {
		this->program_counter = branch();
	}
}

void CPU::BIT(const AddressingMode mode) {
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);
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
	}
}

void CPU::BNE() {
	if ((this->status & Flag::Zero) != 0) {
		this->program_counter = this->branch();
	}
}

void CPU::BPL() {
	if ((this->status & Flag::Zero) == 0) { 
		this->program_counter = this->branch();
	}
}

void CPU::BVC() {
	if ((this->status & Flag::Overflow) == 0) { 
		this->program_counter = this->branch();
	}
}

void CPU::BVS() {
	if ((this->status & Flag::Overflow) != 0) { 
		this->program_counter = this->branch();
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
	// Push program counter and processor status onto the stack
	uint16_t interrupt_vector = 0xFFFE;
	this->program_counter = memory_read(interrupt_vector);
	// Set the break flag to 1
	this->status = this->status | Flag::Break;
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

	memory_write(operand_addres, value-1);
	update_zero_and_negative_flags(value-1);
}

void CPU::DEX() {
	this->register_irx += 1;
	update_zero_and_negative_flags(this->register_irx);
}

void CPU::DEY() {
	this->register_iry += 1;
	update_zero_and_negative_flags(this->register_iry);
}

void CPU::EOR(const AddressingMode mode) {
	const uint16_t operand_addres = get_operand_address(mode);
	const uint8_t value = memory_read(operand_addres);

	this->register_a = this->register_a ^ value;
	update_zero_and_negative_flags(this->register_a);
}

void CPU::INC(const AddressingMode mode) {
	const uint16_t operand_addres = get_operand_address(mode);
	uint8_t value = memory_read(operand_addres);

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
	this->program_counter = address;
}

void CPU::JSR() {
	// Push current program_counter - 1 to the stack as return address.
	const uint16_t return_address = this->program_counter - 1;
	push_stack_uint16(return_address);

	// Get the subroutine address and set the program counter to this address
	const uint16_t address = get_operand_address(AddressingMode::Absolute);
	this->program_counter = address;
} 

void CPU::LDA(const AddressingMode mode) {
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);

	this->register_a = operand;
	update_zero_and_negative_flags(this->register_a);
}

void CPU::LDX(const AddressingMode mode) {
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);

	this->register_a = operand;
	update_zero_and_negative_flags(this->register_a);
}

void CPU::LDY(const AddressingMode mode) {
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);

	this->register_a = operand;
	update_zero_and_negative_flags(this->register_a);
}

uint8_t CPU::LSR(const AddressingMode mode) {
	// Why is it Logical Shift Right but Arithmatic Shift Left???
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);

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

	uint8_t result = operand << 1;

	// Update the 0 bit by using the old Carry flag
	if ((this->status & Flag::Carry) == Flag::Carry) {
		// Flag is set	
		result = result | 0b00000001;
	} 

	// Update the Carry flag to the new value
	if ((operand & 0b10000000) == 0) {
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
	this->program_counter = pop_stack_uint16() - 1;
}

void CPU::SBC(const AddressingMode mode) {
	// [TODO]
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
	memory_write(address, this->register_a);
}

void CPU::STX(const AddressingMode mode) {
	const uint16_t address = get_operand_address(mode);
	memory_write(address, this->register_irx);
}

void CPU::STY(const AddressingMode mode) {
	const uint16_t address = get_operand_address(mode);
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
	// [TODO]
}

void CPU::TXA() {
	this->register_a = this->register_irx;
	update_zero_and_negative_flags(this->register_a);
}

void CPU::TSA() {
	// [TODO]
}

void CPU::TXS() {
	// [TODO]
}

void CPU::TYA() {
	this->register_a = this->register_iry;
}

void CPU::AND(const AddressingMode mode) {
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);

	// Update register and flags
	this->register_a = this->register_a & operand;
	update_zero_and_negative_flags(this->register_a);
}

uint8_t CPU::ASL(const AddressingMode mode) {
	uint16_t operand_adress = get_operand_address(mode);
	uint8_t operand = memory_read(operand_adress);

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
	// [TODO]: refactor this to get jump address using the memory read API and adressing mode
	// Read the label for the jump to be made and jump to the address
	uint8_t jmp = memory_read(this->program_counter);
	uint16_t jmp_addr = this->program_counter + 1 + jmp;

	// Update the program counter
	return jmp_addr;
}

void CPU::compare(const uint8_t reg, const AddressingMode mode) {
	uint16_t operand_address = this->get_operand_address(mode);
	uint8_t operand = memory_read(operand_address);

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
	}

	// Carry if sum is larger then what fits in the 8-bit register
	if (sum > 0xFF) {
		update_flag(Flag::Carry, Mode::Set);
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

			if ((jmp & 0b10000000) != 0) {
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
			return memory_read_uint16(this->program_counter);
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
			uint8_t ptr = base + this->register_iry;

			uint16_t lo_byte = memory_read(ptr);
			uint16_t hi_byte = memory_read(ptr+1);

			return (hi_byte << 8) | lo_byte;
		}
		default: { // Throw an error if the AddressingMode is not in the list
			throw std::runtime_error("Enum element not found");
		}
	}

	return 0;
}

void CPU::hex_dump() {
	const char* cdefault = "\033[0m";
	const char* cyellow = "\033[33m";

	for (int i = 0; i < 0xFFFF; i += 16) {
		std::cout << cyellow << std::setfill('0') << std::setw(4) << std::uppercase << std::hex << i << ": " << cdefault;
		for (int j = i; j < i+16; j++) {
			int val = this->memory[j];
			std::cout << std::setfill('0') << std::setw(2) << std::hex << val << " ";

			if (j-i == 7) {
				// Insert extra space in between bytes
				std::cout << " ";
			}
		}
		std::cout << std::endl;
	}
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
			std::cout << std::setfill('0') << std::setw(2) << std::hex << val << " ";

			if (j-i == 8) {
				// Insert extra space in between bytes
				std::cout << " ";
			}
		}
		std::cout << std::endl;
	}
}

const std::bitset<8> as_binary8(const uint8_t val) {
	return std::bitset<8>(val);
}

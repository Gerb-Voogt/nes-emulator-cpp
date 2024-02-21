#include <bitset>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <iostream>

#include "6502.hpp"

CPU::CPU() {
	register_a = 0;
	register_irx = 0;
	register_iry = 0;
	program_counter = 0;
	status = 0;

	// Initialize memory space to 0
	for (int i = 0; i < 0xFFFF; i++) {
		memory[i] = 0;
	}
}

CPU::~CPU() {
	// All values are stack allocated, no deallocation needs to be taken
	// care off. The values will automatically get resolved once their lifetime ends.
}

uint8_t CPU::memory_read(const uint16_t addr) const {
	return this->memory[addr];
}

uint16_t CPU::memory_read_uint16(const uint16_t addr) const {
	// 8 bit values read into 16 bit variables such that the left most
	// byte for both is padded with zeros
	uint16_t lo_byte = this->memory_read(addr);
	uint16_t hi_byte = this->memory_read(addr+1);

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
	this->memory_write(addr, lo_byte);
	this->memory_write(addr+1, hi_byte);
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

	this->program_counter = this->memory_read_uint16(0xFFFC);
}

void CPU::execute_instruction(const uint8_t opcode) {
	switch (opcode) {
		// brk instruction, do nothing, execution should have terminated before this
		case 0x00: {
		}
		// nop instruction
		case 0xEA: {
			this->nop();
		}
		// lda instructions
		case 0xA9: { // immediate
			this->lda(AddressingMode::Immediate);
			this->program_counter += 1;
		}
		case 0xA5: { // Zero Page
			this->lda(AddressingMode::ZeroPage);
			this->program_counter += 1;
		}
		case 0xB5: { // Zero Page
			this->lda(AddressingMode::ZeroPageX);
			this->program_counter += 1;
		}
		case 0xAD: { // Absolute
			this->lda(AddressingMode::Absolute);
			this->program_counter += 2;
		}
		case 0xBD: { // AbsoluteX
			this->lda(AddressingMode::AbsoluteX);
			this->program_counter += 2;
		}
		case 0xB9: { // AbsoluteY
			this->lda(AddressingMode::AbsoluteY);
			this->program_counter += 2;
		}
		case 0xA1: { // Indirect X
			this->lda(AddressingMode::IndirectX);
			this->program_counter += 1;
		}
		case 0xB1: { // Indirect Y
			this->lda(AddressingMode::IndirectY);
			this->program_counter += 1;
		}
		// tax
		case 0xAA: {  // implied
			this->tax();
		}
		// inx
		case 0xE9: { // implied
			this->inx();
		}
		// iny
		case 0xC8: { // implied
			this->iny();
		}
	}
}

int CPU::interpret(std::vector<uint8_t> program) {
	while (true) {
		uint8_t opcode = program[this->program_counter]; // Fetch the instruction
		this->program_counter += 1;
		this->execute_instruction(opcode);
	}
}

void CPU::run() {
	while (true) {
		uint8_t opcode = this->memory_read(program_counter);
		this->program_counter += 1;
		this->execute_instruction(opcode);
	}
}

void CPU::nop() { }

void CPU::lda(const AddressingMode mode) {
	const uint8_t operand_address = get_operand_address(mode);
	const uint8_t operand = this->memory_read(operand_address);

	this->register_a = operand;
	update_zero_and_negative_flags(this->register_a);
}

void CPU::tax() {
	this->register_irx = this->register_a;
	update_zero_and_negative_flags(this->register_irx);
}

void CPU::inx() {
	this->register_irx = this->register_irx + 1;
	update_zero_and_negative_flags(this->register_irx);
}

void CPU::iny() {
	this->register_iry = this->register_iry + 1;
	update_zero_and_negative_flags(this->register_iry);
}

void CPU::update_zero_and_negative_flags(const uint8_t reg) {
	if (reg == 0) {
		this->status = this->status | 0b00000010; // Set the Z flag
	} else {
		this->status = this->status & 0b11111101; 
	}
	if ((reg & 0b01000000) != 0) {
		this->status = this->status | 0b01000000; // Set the N flag
	} else {
		this->status = this->status & 0b10111111;
	}
}

uint16_t CPU::get_operand_address(const AddressingMode mode) {
	switch(mode) {
		case AddressingMode::Immediate: {
			return this->program_counter;
		}
		case AddressingMode::ZeroPage: {
			return (uint16_t)memory_read(this->program_counter);
		}
		// C++ does wrapping addition by default on uint8 and uint16 types
		case AddressingMode::ZeroPageX: {
			const uint8_t pos = this->memory_read(this->program_counter);
			const uint16_t addr = (uint16_t)(pos + register_irx); 
			return addr;
		}
		case AddressingMode::ZeroPageY: {
			const uint8_t pos = this->memory_read(this->program_counter);
			const uint16_t addr = (uint16_t)(pos + register_iry);
			return addr;
		}
		case AddressingMode::Absolute: {
			return this->memory_read_uint16(this->program_counter); // Read the address
		}
		case AddressingMode::AbsoluteX: {
			uint16_t base = this->memory_read_uint16(this->program_counter);
			uint16_t addr = base + this->register_irx;
			return addr;
		}
		case AddressingMode::AbsoluteY: {
			uint16_t base = this->memory_read_uint16(this->program_counter);
			uint16_t addr = base + this->register_iry;
			return addr;
		}
		case AddressingMode::Indirect: {
			uint16_t ptr = this->memory_read_uint16(this->program_counter); // Read the address
			return this->memory_read_uint16(ptr); // read the data at the ptr location
		}
		case AddressingMode::IndirectX: {
			uint8_t base = this->memory_read(this->program_counter);
			uint8_t ptr = base + this->register_irx;

			uint16_t lo_byte = this->memory_read(ptr);
			uint16_t hi_byte = this->memory_read(ptr+1);

			return (hi_byte << 8) | lo_byte;
		}
		case AddressingMode::IndirectY: {
			uint8_t base = this->memory_read(this->program_counter);
			uint8_t ptr = base + this->register_iry;

			uint16_t lo_byte = this->memory_read(ptr);
			uint16_t hi_byte = this->memory_read(ptr+1);

			return (hi_byte << 8) | lo_byte;
		}
		default: { // Throw an error if the AddressingMode is not in the list
			throw std::runtime_error("Enum element not found");
		}
	}

	return 0;
}

// Debug Functions

// This function should be changed such that it prints out a more readable
// table format rather than dumping all the text to the screen like it does not
void CPU::print_memory_content() {
	// Print out the reserved memory space
	std::cout << "Address 0x0000 - 0x8000" << std::endl;
	for (int i = 0; i < 0x8000; i++) {
		std::cout << unsigned(this->memory[i]) << std::endl;
	}
	std::cout << std::endl << "Address 0x8000 - 0xFFFF" << std::endl;
	for (int i = 0x8000; i < 0xFFFF; i++) {
		std::cout << unsigned(this->memory[i]) << std::endl;
	}
}

const std::bitset<8> as_binary8(const uint8_t val) {
	return std::bitset<8>(val);
}

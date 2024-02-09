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
	for (int i = 0; i < program_length; i++) {
		this->memory[0x8000+i] = program[i]; // load the program into memory
	}
	this->program_counter = 0x8000; // Set the program counter to the first instruction
}


void CPU::reset() {
	this->register_a = 0;
	this->register_irx = 0;
	this->register_iry = 0;
	this->status = 0;

	this->program_counter = this->memory_read_uint16(0xFFFC);
}

int CPU::interpret(std::vector<uint8_t> program) {
	while (true) {
		uint8_t opcode = program[this->program_counter]; // Fetch the instruction
		this->program_counter += 1;

		switch (opcode) {
			case 0x00: {
				return 0;
			}
			case 0xA9: { // lda immediate
				uint8_t param = program[program_counter];
				this->program_counter += 1;
				this->lda(param);
				break;
			}
			case 0xAA: { // tax
				this->tax();
				break;
			}
			case 0xE9: { // inx
				this->inx();
				break;
			}
		}
	}
}

void CPU::lda(const uint8_t param) {
	this->register_a = param;
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
	for (int i = 0x8000+1; i < 0xFFFF; i++) {
		std::cout << unsigned(this->memory[i]) << std::endl;
	}
}

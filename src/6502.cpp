#include <cstdint>
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

void CPU::memory_write(const uint16_t addr, const uint8_t data) {
	this->memory[addr] = data;
}

void CPU::load_program(const std::vector<uint8_t> program) {
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

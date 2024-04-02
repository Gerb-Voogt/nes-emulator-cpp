// [TODO]: Fix the order of the Opcdes in this file to be alphabetical
//	like the declaration order in 6502.hpp
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
	stack_pointer = 0x01FF;
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

	uint16_t first_instruction = 0xFFFC;
	this->program_counter = memory_read_uint16(first_instruction);
}

void CPU::execute_instruction(const uint8_t opcode) {
	// [TODO]: Wrap the instruction in an enum for better matching
	switch (opcode) {
		// brk instruction, execution should be terminated
		case 0x00: {
			break;
		}
		// nop instruction
		case 0xEA: {
			this->NOP();
			break;
		}
		// lda instructions
		case 0xA9: { // immediate
			this->LDA(AddressingMode::Immediate);
			this->program_counter += 1;
			break;
		}
		case 0xA5: { // Zero Page
			this->LDA(AddressingMode::ZeroPage);
			this->program_counter += 1;
			break;
		}
		case 0xB5: { // Zero Page
			this->LDA(AddressingMode::ZeroPageX);
			this->program_counter += 1;
			break;
		}
		case 0xAD: { // Absolute
			this->LDA(AddressingMode::Absolute);
			this->program_counter += 2;
			break;
		}
		case 0xBD: { // AbsoluteX
			this->LDA(AddressingMode::AbsoluteX);
			this->program_counter += 2;
			break;
		}
		case 0xB9: { // AbsoluteY
			this->LDA(AddressingMode::AbsoluteY);
			this->program_counter += 2;
			break;
		}
		case 0xA1: { // Indirect X
			this->LDA(AddressingMode::IndirectX);
			this->program_counter += 1;
			break;
		}
		case 0xB1: { // Indirect Y
			this->LDA(AddressingMode::IndirectY);
			this->program_counter += 1;
			break;
		}
		// tax
		case 0xAA: {  // implied
			this->TAX();
			break;
		}
		// inx
		case 0xE9: { // implied
			this->INX();
			break;
		}
		// iny
		case 0xC8: { // implied
			this->INY();
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
		this->execute_instruction(opcode);
	}
	return 0;
}

void CPU::run() {
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
		this->program_counter = this->branch();
	}
}

void CPU::BEQ() { 
	if ((this->status & Flag::Zero) == Flag::Zero) {
		this->program_counter = this->branch();
	}
}

void CPU::BIT(const uint8_t bitmask, const AddressingMode mode) {
	const uint16_t operand_address = get_operand_address(mode);
	const uint8_t operand = memory_read(operand_address);

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

void CPU::JSR() { } // [TODO]

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
	// [TODO]
}

void CPU::PHP() {
	// [TODO]
}

void CPU::PLA() {
	// [TODO]
}

void CPU::PLP() {
	// [TODO]
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
	// [TODO]
}

void CPU::RTS() {
	// [TODO]
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
			// Add 0x8000, as program does not live in the zero page
			return this->program_counter + 0x8000;
		}
		case AddressingMode::Relative: {
			uint8_t jmp = memory_read(this->program_counter);
			uint16_t jmp_addr = this->program_counter + 1 + jmp;
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
			return memory_read_uint16(this->program_counter); // Read the address
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

// +-----------------+
// | Debug Functions |
// +-----------------+

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

#pragma once
#include <cstdint>
#include <vector>

class CPU {
public:
	uint16_t program_counter;
	uint16_t stack_pointer;
	uint8_t register_a;
	uint8_t register_irx;
	uint8_t register_iry;
	uint8_t status;
	uint8_t memory[0xFFFF]; // Memory space, [0x8000...0xFFFF] reserved for Program ROM

	//@description default Constructor Initializes all CPU registers to 0
	CPU();

	//@description Read memory from address
	//@param uint16_t addr, the address to read
	uint8_t memory_read(const uint16_t addr) const;

	//@description Write memory to address
	//@param uint16_t addr, the address to write
	//@param uint8_t data, the data to write to the address
	void memory_write(const uint16_t addr, const uint8_t data);

	//@description load a program into memory space
	//@param std::vec<uint8_t> program, vector containing the program instructions
	void load_program(const std::vector<uint8_t> program);

	//@description load a program into memory space and run it
	//@param uint16_t addr, the address to write
	//@param std::vec<uint8_t> program, vector containing the program instructions
	void load_program_and_run(const std::vector<uint8_t> program);
	
	//@description Interpret a program (sequence of instructions). Cycle consists of fetching an instruction from the PC address, decoding this
	// instruction and executing this instruction, and repeat. Mainly useful as a debugging interface and for unit testing
	//@param std::vector<uint8_t> Program, sequence of instructions
	int interpret(const std::vector<uint8_t> program);

	//@description Run the CPU, executing whatever program is loaded into the memory space
	void run();
	
	//@description Load Accumulator, loads a byte of memory into the accumulator, setting the Z and N flags
	void lda(const uint8_t param);

	//@description Transfer Accumulator to X, copies current content of the accumulator into the X register, setting Z and N flags
	void tax();
	
	//@description Increment X register, adds one to the X register, setting Z and N flags
	void inx();

	void update_zero_and_negative_flags(const uint8_t reg);

	void print_memory_content();
};

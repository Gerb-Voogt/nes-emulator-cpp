#pragma once
#include <cstdint>
#include <vector>

enum AddressingMode {
	Immediate,
	ZeroPage,
	ZeroPageX,
	ZeroPageY,
	Absolute,
	AbsoluteX,
	AbsoluteY,
	IndirectX,
	IndirectY,
	NoneAdressing
};

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

		//@description Read 2 bytes of memory from address. Apply conversion as the NES CPU uses little endian addressing
		//@param uint16_t addr, the address to read
		uint16_t memory_read_uint16(const uint16_t addr) const;

		//@description Write memory to address
		//@param uint16_t addr, the address to write
		//@param uint8_t data, the data to write to the address
		void memory_write(const uint16_t addr, const uint8_t data);

		//@description Write 2 bytes of memory to the address. Apply conversion as the NES CPU uses little endian addressing
		//@param uint16_t addr, the address to write
		//@param uint16_t data, the data to write to the address
		void memory_write_uint16(const uint16_t addr, const uint16_t data);

		//@description load a program into memory space
		//@param std::vec<uint8_t> program, vector containing the program instructions
		void load_program(const std::vector<uint8_t> program);

		//@description load a program into memory space and run it
		//@param uint16_t addr, the address to write
		//@param std::vec<uint8_t> program, vector containing the program instructions
		void load_program_and_run(const std::vector<uint8_t> program);

		//@description Special subroutine that gets called when a cartridge is inserted
		// resets the state (registers and flags all get set to 0) and sets the program counter
		// to the address 0xFFFC
		void reset();
		
		//@description Interpret a program (sequence of instructions). Cycle consists of fetching an instruction from the PC address, decoding this
		// instruction and executing this instruction, and repeat. Mainly useful as a debugging interface and for unit testing
		//@param std::vector<uint8_t> Program, sequence of instructions
		int interpret(const std::vector<uint8_t> program);

		//@description Run the CPU, executing whatever program is loaded into the memory space
		void run();
		
		//@description Load Accumulator, loads a byte of memory into the accumulator, setting the Z and N flags
		//@param const AddressingMode mode, addressing mode to be used
		void lda(const AddressingMode mode);

		//@description Transfer Accumulator to X, copies current content of the accumulator into the X register, setting Z and N flags
		void tax();
		
		//@description Increment X register, adds one to the X register, setting Z and N flags
		void inx();

		//@description Increment Y register, adds one to the Y register, setting Z and N flags
		void iny();
		//
		//@description NOP, does nothing, this is mainly here to possibly be cycle accurate in the future
		void nop();

		//@description Update the zero and negative flags 
		//@param uint8_t reg, the register used to update
		void update_zero_and_negative_flags(const uint8_t reg);

		//@description Get the address of the operands based on the addressing mode
		//@param AddressingMode mode, The addressingmode being used (immediate, absolute, etc.)
		uint16_t get_operand_address(const AddressingMode mode);

		//@description Dump the memory content to stdout for debugging purposes
		void print_memory_content();
};

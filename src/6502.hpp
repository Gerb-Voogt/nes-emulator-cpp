#pragma once
#include <bitset>
#include <cstdint>
#include <vector>

// Enums for code readability
enum AddressingMode {
	Immediate,
	ZeroPage,
	ZeroPageX,
	ZeroPageY,
	Absolute,
	AbsoluteX,
	AbsoluteY,
	Indirect,
	IndirectX,
	IndirectY,
};

enum Flag {
	Carry = 0b0000001,
	Zero = 0b00000010,
	InteruptDisable = 0b00000100,
	DecimalMode = 0b00001000,
	Break = 0b00010000,
	Overflow = 0b01000000,
	Negative = 0b10000000,
};

// Mode enum for specifying what to do when updating the status register
// Set - Set a specific flag
// Clear - Clear a specific flag
// Update - Negate a specific flag
enum Mode {
	Set,
	Clear,
	Update,
};

// CPU class containing CPU state and all OPCODE subroutines
class CPU {
	public:
		uint16_t program_counter;
		uint16_t stack_pointer;
		uint8_t register_a;
		uint8_t register_irx;
		uint8_t register_iry;
		uint8_t status;

		// This might give a warning for some compilers as a large amount of data 
		// is allocated on the stack. First 256 bytes (0x0100) reserved as the zero page
		// Which has faster access times.
		uint8_t memory[0xFFFF]; // Memory space, [0x8000...0xFFFF] reserved for Program ROM

		//@description default Constructor Initializes all CPU registers to 0
		CPU();
	
		//@description Destructor, clean up the CPU object
		~CPU();

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

		//@description load a program into memory space, reset the registeres and run it
		//@param uint16_t addr, the address to write
		//@param std::vec<uint8_t> program, vector containing the program instructions
		void load_program_and_run(const std::vector<uint8_t> program);

		//@description Special subroutine that gets called when a cartridge is inserted
		// resets the state (registers and flags all get set to 0) and sets the program counter
		// to the address 0xFFFC
		void reset();
		
		//@description execute a specific opcode
		//@param uint8_t opcode, the opcode corresponding to the instruction to be executed
		void execute_instruction(const uint8_t opcode);

		//@description Interpret a program (sequence of instructions). Cycle consists of fetching an instruction from the PC address, decoding this
		// instruction and executing this instruction, and repeat. Mainly useful as a debugging interface and for unit testing
		//@param std::vector<uint8_t> Program, sequence of instructions
		int interpret(const std::vector<uint8_t> program);

		//@description Run the CPU, executing whatever program is loaded into the memory space
		void run();
		
		//@description Add with Carry, Adds content to accumulator together with the carry bit. Carry bit set if overflow occurs
		//@param const AddressingMode mode, addressing mode to be used
		void adc(const AddressingMode mode);

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

		//@description Add an operand to the accumulator, accounting for carryover
		//@param uint8_t operand, the operand to add to the accumulator
		void add_to_accumulator_register(const uint8_t operand);

		//@description Update the zero and negative flags 
		//@param uint8_t reg, the register used to update
		void update_zero_and_negative_flags(const uint8_t reg);

		//@description Update overflow flag
		//@param uint8_t reg, the register used to update the status register
		//@param uint8_t reg, the operand used, used to check if carryover should occur
		void update_carry_flag(const uint8_t reg, const Mode mode);

		//@description Update overflow flag (???)
		void update_overflow_flag();

		//@description Get the address of the operands based on the addressing mode
		//@param AddressingMode mode, The addressingmode being used (immediate, absolute, etc.)
		uint16_t get_operand_address(const AddressingMode mode);

		//@description Dump the memory content to stdout for debugging purposes
		void print_memory_content();
};
 
// Function for debugging and printing purposes. Prints a uint8_t variable as the bitstring representation
const std::bitset<8> as_binary8(const uint8_t val);

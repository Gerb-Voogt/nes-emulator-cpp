#pragma once
#include <bitset>
#include <cstdint>
#include <vector>

// Enums for code readability
enum AddressingMode {
	Immediate,
	Relative,
	Accumulator,
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
	
		// +--------------------------------------------------+
		// | Generic memory interface for reading and writing |
		// +--------------------------------------------------+
	
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

		// +-----------------+
		// | Stack Interface |
		// +-----------------+

		//@description Push a value onto the stack, stack pointer is updated to point to the new top of the stack
		//@param uint8_t value, the value to push onto the stack
		void push_stack(const uint8_t value);

		//@description Pull a value from the stack, stack pointer is updated to point towards the new top of the stack
		//@return uint8_t data, the data the stack pointer in pointing to
		uint8_t pull_stack();

		// +--------------------------------------------+
		// | Program Execution and Instruction Handling |
		// +--------------------------------------------+

		//@description Special subroutine that gets called when a cartridge is inserted
		// resets the state (registers and flags all get set to 0) and sets the program counter
		// to the address 0xFFFC
		void reset();
		
		//@description execute a specific opcode
		//@param uint8_t opcode, the opcode corresponding to the instruction to be executed
		void execute_instruction(const uint8_t opcode);

		//@description Interpret a program (sequence of instructions). Cycle consists of fetching an instruction from the PC address, decoding this
		//  instruction and executing this instruction, and repeat. Mainly useful as a debugging interface and for unit testing
		//@param std::vector<uint8_t> Program, sequence of instructions
		int interpret(const std::vector<uint8_t> program);

		//@description Run the CPU, executing whatever program is loaded into the memory space
		void run();
		
		//@description Add with Carry, Adds content to accumulator together with the carry bit. Carry bit set if overflow occurs
		//@param const AddressingMode mode, addressing mode to be used
		void ADC(const AddressingMode mode);

		//@description Logical and operating on the accumulator.
		//@param const AddressingMode mode, addressing mode to be used. Supported addressing
		//  modes are [Immediate, Zero Page, Zero PageX, Absolute, AbsoluteX, AbsoluteY, IndirectX, IndirectY]
		void AND(const AddressingMode mode);

		//@description Arithmatic shift left
		//@param const AddressingMode mode, addressing mode to be used
		uint8_t ASL(const AddressingMode mode);

		//@description Branch if Carry Clear, branch to a new location if the carry flag is clear
		void BCC();

		//@description Branch if Carry Set, branch to a new location if the carry flag is set
		void BCS();

		//@description Branch if EQual, branch to a new location if the zero flag is set
		void BEQ();

		//@description BIT set, tests if one or more bits are set in a target memory location. The mask pattern A
		//  is ANDed with the value in memory to set or clear the zero flag, however result is not kept.
		//  bits 7 and 6 of the value from memory are copied into the N and V flags.
		//  Updates the status register
		//@param uint16_t bitmask
		//	modes: [ZeroPage, Absolute]
		void BIT(const uint8_t bitmask, const AddressingMode mode);

		//@description Branch if MInus, branch to a new location if the negative flag is set
		void BMI();
	
		//@description Branch if Not Equal, branch to a new location if the zero flag is clear
		void BNE();

		//@description Branch if Positive, branch to a new location if the negative flag is clear
		void BPL();

		//@description Branch if oVerflow Clear
		void BVC();

		//@description Branch if oVerflow Set
		void BVS();

		//@description CLear Carry, sets the Carry flag to 0
		void CLC();

		//@description CLear Decimal, sets the decimal flag to 0
		void CLD();

		//@description CLear Interrupt disable, sets the intterupt disable flag to 0
		void CLI();

		//@description Force Interrupt, forces the generation of an interupt request. Program counter and processor status
		//	are pushed to the stack, then the IRQ interrupt vector at $FFFE/F is loaded into the PC and the break flag
		//	is set to 1
		void BRK();

		//@description CoMPare accumulator, compares memory held at specified address against the accumulator. 
		//	Carry flag set if A >= M, Zero-Flag set if A == M. Sets the negative bit if the result is negative.
		//	CMP basically sets flags as if a subtraction happened.
		//@param const AddressingMode mode
		//	modes: [..., ..., ...] [TODO]: Fill this in
		void CMP(const AddressingMode mode);

		//@description ComPare X register, compares memory held at specified address against the X-register. 
		//	Carry flag set if A >= M, Zero-Flag set if A == M. Sets the negative bit if the result is negative.
		//	CMP basically sets flags as if a subtraction happened.
		//@param const AddressingMode mode
		//	modes: [..., ..., ...] [TODO]: Fill this in
		void CPX(const AddressingMode mode);

		//@description ComPare Y register, compares memory held at specified address against the Y-register. 
		//	Carry flag set if A >= M, Zero-Flag set if A == M. Sets the negative bit if the result is negative.
		//	CMP basically sets flags as if a subtraction happened.
		//@param const AddressingMode mode
		//	modes: [..., ..., ...] [TODO]: Fill this in
		void CPY(const AddressingMode mode);

		//@description DECrement memory, subtracts one from the value held at a specific location in memory.
		//@param AddressingMode mode
		void DEC(const AddressingMode mode);

		//@description DEcrement X-register, subtracts one
		void DEX();

		//@description DEcrement Y-register, subtracts one
		void DEY();

		//@description Exclusive OR, reads a value from and provides an exclusive or against the accumulator
		void EOR(const AddressingMode mode);

		//@description INCrement memory, increment the memory located at specified address, setting Z and N flags
		void INC(const AddressingMode mode);

		//@description INcrement X register, increment the memory located at specified address, setting Z and N flags
		void INX();

		//@description INcrement Y register, increment the memory located at specified address, setting Z and N flags
		void INY();

		//@description JuMP, sets the program counter to the address specified
		//@param `const AddressingMode mode`, addressing mode to be used
		void JMP(const AddressingMode mode);

		//@description Jump to SubRoutine, pushes the address minus one of the return point on the stack and then
		// sets the program counter to the target memory address
		// [TODO]
		void JSR();

		//@description Load Accumulator, loads a byte of memory into the accumulator, setting the Z and N flags
		//@param const AddressingMode mode, addressing mode to be used
		void LDA(const AddressingMode mode);

		//@description LoaD X register, loads a byte of memory into the x register, setting the Z and N flags
		//@param const AddressingMode mode, addressing mode to be used
		void LDX(const AddressingMode mode);

		//@description LoaD Y register, loads a byte of memory into the y register, setting the Z and N flags
		//@param const AddressingMode mode, addressing mode to be used
		void LDY(const AddressingMode mode);

		//@description Logical Shift Right, each bit in A or M is shifted one place to the right. bit 0 gets shifted into the
		//	carry. Bit 7 is set to 0. Updates Z and N Flags.
		//@param const AddressingMode mode, addressing mode to be used
		uint8_t LSR(const AddressingMode mode);

		//@description logical inclusive OR Accumulator, bitwise OR operation on the Acummulator and some other operand
		//	specified through addressingmode. Updates Z and N flags.
		void ORA(const AddressingMode mode);

		//@description PusH Accumulator, pushes a copy of the accumulator onto the stack
		// [TODO]
		void PHA();

		//@description PusH Processor status, pushes a copy of the status flags onto the stack
		// [TODO]
		void PHP();

		//@description PulL Accumulator, pulls an 8-bit value from the stack onto the accumulator
		// [TODO]
		void PLA();

		//@description PulL Processor status, pull status flags from the stack
		// [TODO]
		void PLP();

		//@description ROtate Left, Move each bit in either the accumulator or in memory one bit to the left. The value from 
		//	the carry bit gets set to bit 0 and the old bit 7 becomes the new carry flag. Difference between this and ASL is
		//	is that ASL does not load in the carry value while this does.
		// Flags: C, Z, N
		void ROL(const AddressingMode mode);

		//@description ROtate Right, Move each bit in either the accumulator or in memory one bit to the right. The value from 
		//	the carry bit gets set to bit 7 and the old bit 0 becomes the new carry flag. Difference between this and LSR is
		//	is that LSR does not load in the carry value while this does.
		// Flags: C, Z, N
		void ROR(const AddressingMode mode);

		//@description ReTurn from Interrupt, pulls processor status and program counter from the stack
		// [TODO]
		void RTI();

		//@description ReTurn from Subroutine, pulls the program counter - 1 from the stack
		// [TODO]
		void RTS();

		//@description SuBtract with Carry, subtracts the content from a specified memory location from the accumulator together with
		//	the negation of the carry bit. If overflow occurs the carry bit is clear, this enables multi-byte subtractions be done
		void SBC(const AddressingMode mode);

		//@description SEt Carry, sets the Carry flag to 1,
		void SEC();
		
		//@description SEt Decimal flag, sets the decimal flag to 1
		void SED();

		//@description SEt Interrupt disable flag, sets the interrupt disable flag to 1
		void SEI();

		//@description STore Accumulator, stores the content of the accumulator to a specified memory address
		void STA(const AddressingMode mode);

		//@description STore X register, stores the content of the x register to a specified memory address
		void STX(const AddressingMode mode);

		//@description STore Accumulator, stores the content of the y register to a specified memory address
		void STY(const AddressingMode mode);

		//@description Transfer Accumulator to X, copies current content of the accumulator into the X register,
		//	setting Z and N flags
		void TAX();

		//@description Transfer Accumulator to Y, copies current content of the accumulator into the Y register,
		//	setting Z and N flags
		void TAY();

		//@description NOP, does nothing, this is mainly here to possibly be cycle accurate in the future
		void NOP();

		//@description branch to a new location
		//@return uint16_t jmp_addr, address to jump to
		uint16_t branch();

		//@description Compare an operand and register value
		//	flags: Z,N,V
		void compare(const uint8_t reg, const AddressingMode mode);

		//@description Add an operand to the accumulator, accounting for carryover. Note that this
		//		function may update the overflow flag when overflow happens
		//@param uint8_t operand, the operand to add to the accumulator
		void add_to_accumulator_register(const uint8_t operand);

		//@description Update the zero and negative flags 
		//@param uint8_t reg, the register used to update
		void update_zero_and_negative_flags(const uint8_t reg);

		//@description Update the flag
		//@param const Flag flag, the flag to operate on
		//@param const Mode mode, the operation to take, can be set, clear and update
		void update_flag(const Flag flag, const Mode mode);

		//@description Get the address of the operands based on the addressing mode
		//@param AddressingMode mode, The addressingmode being used (immediate, absolute, etc.)
		uint16_t get_operand_address(const AddressingMode mode);

		//@description Dump the memory content to stdout for debugging purposes
		void print_memory_content();
};
 
// Function for debugging and printing purposes. Prints a uint8_t variable as the bitstring representation
const std::bitset<8> as_binary8(const uint8_t val);

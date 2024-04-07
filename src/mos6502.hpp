#pragma once
#include <bitset>
#include <cstdint>
#include <vector>
#include <map>

/**
 * AddressingMode enum for code readability
 */
enum AddressingMode {
    Implied,
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

/**
 * Flag enum for easily accessing and updating specific status register flags
 */
enum Flag {
    Carry = 0b0000001,
    Zero = 0b00000010,
    InteruptDisable = 0b00000100,
    DecimalMode = 0b00001000,
    Break = 0b00010000,
    Overflow = 0b01000000,
    Negative = 0b10000000,
};

/**
 * Mode enum for Setting and Clearing status flags
 */
enum Mode {
    Set,
    Clear,
    Update,
};

/**
 * Structure for storing Opcodes and their associated meta-data.
 */
struct Opcode {
    const uint16_t code;
    const uint8_t size;
    const uint8_t cycles;
    const AddressingMode mode;
    const std::string name;

    /**
     * Constructor, create an opcode with associated code, size, addressingmode, cycle count and name
     * ---
     * @param `const uint16_t code` the number (typically hex) associated with the opcode
     * @param `const uint8_t size` the amount of bytes the opcode uses. This will be used to appropriately increase the program counter
     * @param `const uint8_t cycles` the amount of cycles that the opcode uses to execute
     * @param `const AddresssingMode mode` the addressingmode associated with the opcode
     * @param `const std::string name` the mnemonic of the opcode
     */
    Opcode(const uint16_t code, const uint8_t size, const uint8_t cycles, const AddressingMode mode, const std::string name);
};

/**
 * Global hashmap of all the available Opcodes and their data
 */
extern std::map<uint16_t, Opcode> OPCODES;


/**
 * 6502 CPU Emulator containing GP registers, a status registers, memory space, a program counter and a stack pointer.
 *
 * The general purpose registers consist of the accumulator (`register_a`), the x register (`register_irx`) and the y
 * register (`register_iry`). Registers can be used for any purpose, with x and y commonly being used for loop-counters
 * as well as setting memory offsets.
 *
 * The program counter stores the offset for the next instruction, the stack pointer points to the next empty address on
 * the stack.
 *
 * ---
 *
 * The status register is used to for storing any flags that may be set by the program. These flags include the following:
 *
 *      - `0b10000000` Negative (N)
 *      - `0b01000000` Overflow (V)
 *      - `0b00100000` Unused
 *      - `0b00010000` Break (B)
 *      - `0b00001000` DecimalMode (D)
 *      - `0b00000100` InteruptDisable (I)
 *      - `0b00000010` Zero (Z)
 *      - `0b00000001` Carry (C)
 *
 * ---
 *
 *  TODO: Add more relevant values to the CPU memory map, including mirrors, PPU registers and APU registers
 *
 *  The memory is represented as a stack allocated array with `0xFFFF` elements. The following ranges are of special note:
 *
 *      - `0x0000` - `0x0100` (256 B), The Zero-Page
 *      - `0x0100` - `0x01FF` (256 B), The stack
 *      - `0x6000` - `0x7FFF` (4 kB), Cartridge RAM (when present)
 *      - `0x8000` - `0xFFFF` (16 kB), The cartridge ROM and mapper registers
 */
class CPU {
public:
    uint16_t program_counter;
    uint8_t stack_pointer;
    uint8_t register_a;
    uint8_t register_irx;
    uint8_t register_iry;
    uint8_t status;
    uint32_t cycles;

    // This might give a warning for some compilers as a large amount of data 
    // is allocated on the stack. First 256 bytes (0x0100) reserved as the zero page
    // Which has faster access times.
    uint8_t memory[0xFFFF];

    /**
     * Default constructor, initialize the PC, SP, registers, status register and memory space to all zeros
     */
    CPU();

    /**
     * Read memory from a specified address.
     *
     * The address is passed as a `uint16_t` and is directly read from the `CPU.memory` array.
     * ---
     * @param `const uint16_t addr`, the address to be read.
     * ---
     * @return `uint8_t result`, the value stored in memory at `addr`
     * ---
     */
    uint8_t memory_read(const uint16_t addr) const;

    /**
     * Read 2 bytes of memory from the specified address. Applies conversion to the endianness as 
     * the 6502 is little endian while `C++` assumes a big endian representation.
     * ---
     * @param `const uint16_t addr`, the address to be read.
     * ---
     * @return `uint16_t result`, the value stored in memory at `addr` with the low byte at `addr`, and the high byte at `addr+1`
     * ---
     */
    uint16_t memory_read_uint16(const uint16_t addr) const;

    /**
     * Write a byte to the specified address.
     * ---
     * @param `const uint16_t addr`, the address to write to
     * @param `const uint8_t data`, the data to be written to this address
     * ---
     */
    void memory_write(const uint16_t addr, const uint8_t data);

    /**
     * Write 2 bytes of memory to the specified address. Applies conversion to the endianess as the 6502
     * is little endian while `C++` assumes big endian representation.
     * ---
     * @param `const uint16_t addr`, the address to write to
     * @param `const uint16_t data`, the 2 bytes of data to be written to this address
     * ---
     */
    void memory_write_uint16(const uint16_t addr, const uint16_t data);

    /**
     * Load a program to the memory space reserved to cartridge ROM. The program gets written to the range
     * `0x8000` - `0xFFFF` of the `CPU.memory` array.
     * ---
     * @param `const std::vector<uint8_t> program`, the vector containing the ordered list of instructions of the program
     * @param `const uint16_t data`, the 2 bytes of data to be written to this address
     * ---
     * @exception `std::out_of_range`, Throws out of range error in the case that the length of the vector exceeds what fits into the memory
     * @exception `std::out_of_range`, Thrown when zero length program is passed in
     * ---
     */
    void load_program(const std::vector<uint8_t> program);

    /**
     * Load a program to the memory space reserved to cartridge ROM and immediately execute it. The program gets written 
     * to the range `0x8000` - `0xFFFF` of the `CPU.memory` array.
     * ---
     * @param `const std::vector<uint8_t> program`, the vector containing the ordered list of instructions of the program to be loaded into memory
     * ---
     */
    void load_program_and_run(const std::vector<uint8_t> program);

    /**
     * Pushes a value onto the stack. The data is placed at the location of the stack pointer which points to the next free location.
     * Note that this operation decrements the stack pointer by 1. The stack starts at `0x01FF` and grows downward towards
     * `0x0100`.
     *
     * TODO: Implement handling of stack over- and underflow exceptions. Not sure how this should be handled by the CPU, look into this
     * ---
     * @param `const uitn16_t value`, 1 byte value to write to the stack
     * ---
     */
    void push_stack(const uint8_t data);

    /**
     * Pushes a 2 bytes value onto the stack, using appropriate endian conversion. The data is placed at the location of the 
     * stack pointer and the stack pointer decremented by 1. which points to the next free location.
     * Note that this operation decrements the stack pointer by 2. The stack starts at `0x01FF` and grows downward towards
     * `0x0100`.
     * ---
     * @param `const uitn16_t value`, 2 byte value to write to the stack
     * ---
     */
    void push_stack_uint16(const uint16_t data);

    /**
     * Pop a 1 byte value from the stack and return it. Stack pointer is updated to point to the new next item after the top
     * ---
     * @return `uint8_t top` the 1 byte value stored at the top of the stack (stack pointer - 1)
     * ---
     */
    uint8_t pop_stack();

    /**
     * Pop a 2 byte value from the stack and return it. Stack pointer is updated to point to the new next item after the top
     * ---
     * @return `uint16_t top` the 2 byte value stored at the top of the stack (stack pointer - 1)
     * ---
     */
    uint16_t pop_stack_uint16();

    /**
     * A special subroutine that gets called when a cartridge is inserted (and hence when program gets loaded). Resets
     * the state of the CPU registers and sets the program counter to `0xFFFC`.
     * ---
     */
    void reset();
    
    /**
     * Execute the OPCODE passed in.
     * ---
     *  @param `uint8_t opcode`, the numerical value corresponding to the opcode to be executed
     * ---
     */
    void execute_instruction(const uint8_t opcode);

    /**
     * Interpret a program being passed in as an argument, without loading it into memory. Cycle consists of fetching an instruction
     * from the address that the PC points to, decoding the instruction and executing it. This function is mainly
     * for debugging and testing
     * ---
     *  @param `uint8_t opcode`, the numerical value corresponding to the opcode to be executed
     * ---
     *  @return `int exit_code`, returns an exit code to signify whether the function ran succesfully (0) or not (-1).
     * ---
     */
    int interpret(const std::vector<uint8_t> program);

    /**
     * Run the CPU, executing whatever program is loaded into memory space `0x8000` - `0xFFFF`
     *
     * TODO: Implementation is currently not working as intented
     * ---
     */
    void run();
    
    /**
     * ADd with Carry, adds the operand to the accumulator along with the carry bit. Carry bit gets set if the addition operation 
     * results in overflow.
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    void ADC(const AddressingMode mode);

    /**
     * logical AND, logical end between operand and accumulator. 
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    void AND(const AddressingMode mode);

    /**
     * Arithmatic Shift Left, shift the content of the operand one bit to the left and return it. This effectively multiplies the 
     * number by 2. If the addressing mode is Accumulator, than the result is stored in the accumulator otherwise it is written 
     * to the memory location it was read from.
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     * @return `uint8_t result`, the result of the left shift 
     * ---
     */
    uint8_t ASL(const AddressingMode mode);

    /**
     * Branch if Carry Clear, branch to a new location if the Carry flag is clear
     * ---
     */
    void BCC();

    /**
     * Branch if Carry Set, branch to a new location if the Carry flag is set
     * ---
     */
    void BCS();

    /**
     * Branch if EQual, branch to a new location if the zero flag is set
     * ---
     */
    void BEQ();

    /**
     * BIT set, tests if one or more bits are set in a register or target memory location. The mask pattern stored in 
     * the accumulator is and-ed with the value in memory to set or clear the zero flag, however the result is not kept. 
     * Bits 7 and 6 from the value in memory are copied into the N and V flags.
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    void BIT(const AddressingMode mode);

    /**
     * Brach if MInus, branch to a new location if the negative flag is set
     * ---
     */
    void BMI();

    /**
     * Branch if Not Equal, branch to a new location if the zero flag is clear
     * ---
     */
    void BNE();

    /**
     * Brach if Positive, branch to a new location if the negative flag is clear
     * ---
     */
    void BPL();

    /**
     * BReaK, forces the generation of an interupt request. Program counter and processor status are pushed to the stack,
     * then the IRQ interrupt vector at 0xFFFE/F is loaded into the PC and the break flag is set.
     * ---
     */
    void BRK();

    /**
     * Brach if oVerflow Clear, branch to a new location if the overflow flag is clear
     * ---
     */
    void BVC();

    /**
     * Brach if oVerflow Set, branch to a new location if the overflow flag is set
     * ---
     */
    void BVS();

    /**
     * CLear Carry, clears the carry flag
     * ---
     */
    void CLC();

    /**
     * CLear Decimal mode, clears the decimal mode flag (decimal mode unused for the NES)
     * ---
     */
    void CLD();

    /**
     * CLear Interrupt disable
     * ---
     */
    void CLI();

    /**
     * CLear oVerflow flag
     * ---
     */
    void CLV();

    /**
     * CoMPare accumulator, compares the accumulator against memory in the location specified by addressingmode. Carry flag
     * is set if A >= M, Zero-flag gets set if A == M. Sets the negative bit if the result is negative. The flags for 
     * this instruction gets set as if subtraction where carried out.
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    void CMP(const AddressingMode mode);

    /**
     * ComPare X-register, compares the x-register against memory in the location specified by addressingmode. Carry flag
     * is set if A >= M, Zero-flag gets set if A == M. Sets the negative bit if the result is negative. The flags for 
     * this instruction gets set as if subtraction where carried out.
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    void CPX(const AddressingMode mode);

    /**
     * ComPare Y-register, compares the y-register against memory in the location specified by addressingmode. Carry flag
     * is set if A >= M, Zero-flag gets set if A == M. Sets the negative bit if the result is negative. The flags for 
     * this instruction gets set as if subtraction where carried out.
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    void CPY(const AddressingMode mode);

    /**
     * DECrement memory, subtracts one from the value held at location specified through addressingmode.
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    void DEC(const AddressingMode mode);

    /**
     * DEcrement X-register, subtracts one from the value held in the x-register
     * ---
     */
    void DEX();

    /**
     * DEcrement Y-register, subtracts one from the value held in the y-register
     * ---
     */
    void DEY();

    /**
     * Exclusive OR, does an XOR between the accumulator and memory held at location specified by adressingmode
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    void EOR(const AddressingMode mode);

    /**
     * INCrement memory, adds one to the value held at location specified through addressingmode.
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    void INC(const AddressingMode mode);

    /**
     * INcrement X-register, adds one to the value held in the x-register
     * ---
     */
    void INX();

    /**
     * INcrement Y-register, adds one to the value held in the y-register
     * ---
     */
    void INY();

    /**
     * JuMP, sets the program at the address specified via the addressingmode.
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    void JMP(const AddressingMode mode);

    /**
     * Jump to SubRoutine, pushes address minus one of the return point on the stack and then sets the program counter to 
     * the target memory address
     * ---
     */
    void JSR();

    /**
     * LoaD Accumulator, loads a byte of memory into the accumulator, setting the zero and negative flags
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    void LDA(const AddressingMode mode);

    /**
     * LoaD X-register, loads a byte of memory into the x-register, setting the zero and negative flags
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    void LDX(const AddressingMode mode);

    /**
     * LoaD Y-register, loads a byte of memory into the y-register, setting the zero and negative flags
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    void LDY(const AddressingMode mode);

    /**
     * Logical Shift Right, each bit in specified memory address or in the accumulator is shifted one spot to the
     * right, setting the zero and negative flags. This essentially halving the data stored at address.
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     * @return `uint8_t result`, the result of the right shift 
     * ---
     */
    uint8_t LSR(const AddressingMode mode);

    /**
     * logical inclusive OR Accumulator, bitwise or operation on the accumulator with an operand specified via
     * addressingmode.
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    void ORA(const AddressingMode mode);


    /**
     * PusH Accumulator, pushes a copy of the accumulator onto the stack
     * ---
     */
    void PHA();

    /**
     * PusH Processor status, pushes a copy of the status register onto the stack
     * ---
     */
    void PHP();

    /**
     * PulL Accumulator, pull the accumulator from the stack
     * ---
     */
    void PLA();

    /**
     * PulL Processor status, pulls the status register from the stack
     * ---
     */
    void PLP();

    /**
     * ROtate Left, Move each bit in either the accumulator or in memory one bit to the left. The value from 
     * the carry bit gets set to bit 0 and the old bit 7 becomes the new carry flag. Difference between this and ASL
     * is that ASL does not load in the carry value while this does.
     *
     * TODO
     * ---
     */
    void ROL(const AddressingMode mode);

    /**
     * ROtate Right, Move each bit in either the accumulator or in memory one bit to the right. The value from 
     * the carry bit gets set to bit 7 and the old bit 0 becomes the new carry flag. Difference between this and LSR is
     * s that LSR does not load in the carry value while this does.
     *
     * TODO
     * ---
     */
    void ROR(const AddressingMode mode);

    /**
     * ReTurn from Interrupt, pulls processor status and program counter from the stack
     *
     * TODO
     * ---
     */
    void RTI();

    /**
     * ReTurn from Subroutine, pulls the program counter - 1 from the stack
     *
     * TODO
     * ---
     */
    void RTS();

    /**
     * SuBtract with Carry, subtracts the content from a specified memory location from the accumulator together with
     * the negation of the carry bit. If overflow occurs the carry bit is clear, this enables multi-byte
     * subtractions be done
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    void SBC(const AddressingMode mode);

    /**
     * SEt Carry flag
     * ---
     */
    void SEC();
    
    /**
     * SEt Decimal mode flag (decimal mode ununsed for NES)
     * ---
     */
    void SED();

    /**
     * SEt Interrupt disable flag
     * ---
     */
    void SEI();

    /**
     * STore Accumulator, stores the content of the accumulator to a specified memory address
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    void STA(const AddressingMode mode);

    /**
     * STore X-register, stores the content of the x-register to a specified memory address
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    void STX(const AddressingMode mode);

    /**
     * STore Y-register, stores the content of the y-register to a specified memory address
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    void STY(const AddressingMode mode);

    /**
     * Transfer Accumulator to X, copies current content of the accumulator into the x register,
     * setting zero and negative flags
     * ---
     */
    void TAX();

    /**
     * Transfer Accumulator to Y, copies current content of the accumulator into the y register,
     * setting zero and negative flags
     * ---
     */
    void TAY();

    /**
     * Transfer Stack pointer to X register, setting zero and negative flags
     * ---
     */
    void TSX();

    /**
     * Transfer X register to Accumulator, setting zero and negative flags
     * ---
     */
    void TXA();

    /**
     * Transfer Stack pointer to Accumulator register, setting zero and negative flags
     * ---
     */
    void TSA();

    /**
     * Transfer X register to Stack pointer, setting zero and negative flags
     * ---
     */
    void TXS();

    /**
     * Transfer Y register to Accumulator, setting zero and negative flags
     * ---
     */
    void TYA();

    /**
     * No OPeration, does nothing, this function exists for cycle accuracy
     * ---
     */
    void NOP();

    /**
     * Branch to a new location. Fetch the address to branch to.
     * ---
     * @return `uint16_t jmp_addr`, address to jump to
     * ---
     */
    uint16_t branch();

    /**
     * Compare an operand and register value, setting zero, negative and overflow flags
     * ---
     */
    void compare(const uint8_t reg, const AddressingMode mode);

    /**
     * Add a value to the accumulator accounting for carry-over, setting the overflow flag if overflow
     * occurs.
     * ---
     * @param `uint8_t operand`, the operand to add to the accumulator
     * ---
     */
    void add_to_accumulator_register(const uint8_t operand);

    /**
     * Common subroutine of setting the zero flag if result is zero and setting the negative flag if the
     * result is negative.
     * ---
     * @param `uint8_t reg`, the register (1 byte value) to be used to update the flags
     * ---
     */
    void update_zero_and_negative_flags(const uint8_t reg);

    /**
     * Sets, clears or flips (updates) the bit in the status register as specified by `flag` and `mode`j
     * ---
     * @param `const Flag flag`, the status flag to act on 
     * @param `const Mode mode`, the action to take (update, set or clear)
     * ---
     */
    void update_flag(const Flag flag, const Mode mode);

    /**
     * Get the address of some operand in memory based on the addressingmode,
     * ---
     * @param `const AddressingMode mode`, the addressing mode to be used for fetching the operand.
     * ---
     */
    uint16_t get_operand_address(const AddressingMode mode);

    /**
     * Dump the memory content to stdout for debugging purposes
     * ---
     */
    void hex_dump();

    /**
     * Overload for hex dump, DUmps the memory content to stdout for a specified range of adresses. Note
     * that the upper_bound and lower_bound need to be multiples of 16. If they are not, they will be rounded to the closest
     * multiples of 16.
     * ---
     * @param `int lower_bound`, the lower bound of the address range to start dumping, needs to be a multiple of 16
     * @param `int upper_bound`, the upper bound of the address range to dump, needs to be a multiple of 16
     * ---
     */
    void hex_dump(int lower_bound, int upper_bound);
    
    /**
     * Alias of `CPU::hex_dump(0x0100, 0x0200)` for convenience. Does a hex dump of the current contents of the stack
     * ---
     */
    void hex_dump_stack();

    /**
     * Alias of `CPU::hex_dump(0x0600, 0x0700)` for convenience. Does a hex dump of the current contents of the stack
     * ---
     */
    void hex_dump_rom();
};

/**
* Function for debugging and printing purposes. Prints a uint8_t variable as the bitstring representation
* ---
* @param `const uint8_t val`, the 1 byte value to be printed as a binary string
* ---
* @return `std::bitset<8> result`, the input argument as a bitstring
* ---
*/
const std::bitset<8> as_binary8(const uint8_t val);

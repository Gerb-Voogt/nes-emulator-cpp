#pragma once
#include <map>
#include <string>

/**
 * Structure for storing Opcodes and their associated meta-data.
 */
class Opcode {
    short code;
    short size;
    short cycles;
    AddressingMode mode;
    std::string name;

    /**
     * Constructor, create an opcode with associated code, size, addressingmode, cycle count and name
     * ---
     * @param `const short code` the number (typically hex) associated with the opcode
     * @param `const short size` the amount of bytes the opcode uses. This will be used to appropriately increase the program counter
     * @param `const short cycles` the amount of cycles that the opcode uses to execute
     * @param `const AddresssingMode mode` the addressingmode associated with the opcode
     * @param `const std::string name` the mnemonic of the opcode
     * ---
     */
    Opcode(short code,
           short size,
           short cycles,
           AddressingMode mode,
           std::string name);
};

const std::map<int, Opcode> create_opcode_table();



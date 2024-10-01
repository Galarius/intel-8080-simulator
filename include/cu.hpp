//
//  cu.hpp
//
//  Created by Ilia Shoshin on 29.09.24.
//

#pragma once

#include <systemc>

namespace sim {

class ControlUnit final : sc_core::sc_module {

public:
    static const uint8_t OP_REG_B;
    static const uint8_t OP_REG_C;
    static const uint8_t OP_REG_D;
    static const uint8_t OP_REG_E;
    static const uint8_t OP_REG_H;
    static const uint8_t OP_REG_L;
    static const uint8_t OP_REG_M;
    static const uint8_t OP_REG_A;

    static const uint8_t OP_GROUP_DATA_TRANSFER;
    static const uint8_t OP_GROUP_MOV;      // MOV and HLT (= MOV M,M)
    static const uint8_t OP_GROUP_ALU;
    static const uint8_t OP_GROUP_SPECIAL;  // ALU Immediate, Branch, Stack, I/O and Machine Contro

    static const uint8_t OP_INST_NOP;
    static const uint8_t OP_INST_HLT;

    sc_core::sc_in<bool> clock;                         // Clock signal

    // Ports
    sc_core::sc_out<sc_dt::sc_uint<8>> aluAccumulator;  // ALU argument 1 (A register)
    sc_core::sc_out<sc_dt::sc_uint<8>> aluOperand;      // ALU argument 2 (argument)
    sc_core::sc_out<sc_dt::sc_uint<4>> aluOpcode;       // Operation code to ALU
    sc_core::sc_in<sc_dt::sc_uint<8>>  aluResult;       // Result from ALU
    sc_core::sc_in<sc_dt::sc_uint<5>>  aluFlags;        // Input flags from ALU

    // Memory ports
    sc_core::sc_out<sc_dt::sc_uint<16>> memAddress;     // Address for memory
    sc_core::sc_out<sc_dt::sc_uint<8>> memDataIn;       // Data to write to memory
    sc_core::sc_in<sc_dt::sc_uint<8>> memDataOut;       // Data read from memory
    sc_core::sc_out<bool> memRead;                      // Read signal
    sc_core::sc_out<bool> memWrite;                     // Write signal

    void signalToReadAt(sc_dt::sc_uint<16> address);
    void waitFor(int);
    void execute(); // Method to manage the control logic
    void inspect();

    ControlUnit(sc_core::sc_module_name name);

private:
    sc_dt::sc_uint<8> getRegisterValue(uint8_t regCode);

    sc_dt::sc_uint<16> pc; // Program counter
    sc_dt::sc_uint<16> sp; // Stack pointer
    sc_dt::sc_uint<5> flags;
    // Registers
    sc_dt::sc_uint<8> accumulator;
    sc_dt::sc_uint<8> registerB;
    sc_dt::sc_uint<8> registerC;
    sc_dt::sc_uint<8> registerD;
    sc_dt::sc_uint<8> registerE;
    sc_dt::sc_uint<8> registerH;
    sc_dt::sc_uint<8> registerL;
};

} // namespace sim

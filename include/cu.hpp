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

    void execute(); // Function to manage the control logic

    ControlUnit(sc_core::sc_module_name name);

private:
    void fetchInstruction();
    void decodeInstruction();
    void executeInstruction();

    sc_dt::sc_uint<8> pc; // 8-bit program counter
};

} // namespace sim

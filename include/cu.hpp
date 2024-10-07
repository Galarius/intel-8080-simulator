//
//  cu.hpp
//
//  Created by Ilia Shoshin on 29.09.24.
//

#pragma once

#include "reg.hpp"

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
    static const uint8_t OP_MEM;
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

    // Bus ports
    sc_core::sc_out<sc_dt::sc_uint<16>> addressBus;
    sc_core::sc_out<sc_dt::sc_uint<8>>  dataBusOut;
    sc_core::sc_in<sc_dt::sc_uint<8>>   dataBusIn;

    // MUX ports
    sc_core::sc_in<sc_dt::sc_uint<8>>  inputMux;       // Input signal from mux
    sc_core::sc_out<sc_dt::sc_uint<8>> outputMux;      // Output signal
    
    // Control Lines
    sc_core::sc_out<bool> memoryWriteEnable;
    sc_core::sc_out<bool> muxWriteEnable;
    sc_core::sc_out<bool> memoryReadEnable;
    sc_core::sc_out<bool> muxReadEnable;

    // MUX ports
    sc_core::sc_out<sc_dt::sc_uint<8>> muxSelect;                 // Select signal for multiplexer

    sc_dt::sc_uint<8> readReg(sc_dt::sc_uint<8> source);
    sc_dt::sc_uint<8> readMemAt(sc_dt::sc_uint<16> address);
    void writeReg(sc_dt::sc_uint<8> source, sc_dt::sc_uint<8> value);
    void writeMemAt(sc_dt::sc_uint<16> address, sc_dt::sc_uint<8> value);
    void waitFor(int);
    void execute(); // Method to manage the control logic

    ControlUnit(sc_core::sc_module_name name);

private:
    sc_dt::sc_uint<8> getRegisterValue(uint8_t regCode);

    sc_dt::sc_uint<16> pc; // Program counter
    sc_dt::sc_uint<16> sp; // Stack pointer
    sc_dt::sc_uint<5> flags;
};

} // namespace sim

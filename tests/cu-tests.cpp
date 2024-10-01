//
//  alu-tests.cpp
//
//  Created by Ilia Shoshin on 28.09.24.
//

#include <systemc>
#include <gtest/gtest.h>

#include "alu.hpp"
#include "cu.hpp"
#include "memory.hpp"
#include "modules.hpp"

using namespace sc_core;
using namespace sim;

static constexpr size_t MemorySize = 65536; // 64 KB

class ControlUnitConsumer final {
    ControlUnit cu{"ControlUnit"};
    Memory<MemorySize> memory{"CU_Memory"};
    ALU alu{"CU_ALU"};
    sc_clock clock {"clock", 0.5, SC_US};    // Configure clock to have 0.5 microsecond period (2 MHz)
public:
    // Signal declarations
    sc_core::sc_signal<sc_dt::sc_uint<8>> memDataOut;
    sc_core::sc_signal<sc_dt::sc_uint<8>> memDataIn;
    sc_core::sc_signal<sc_dt::sc_uint<16>> memAddress;
    sc_core::sc_signal<bool> memRead;
    sc_core::sc_signal<bool> memWrite;
    
    sc_core::sc_signal<sc_dt::sc_uint<8>> aluAccumulator;
    sc_core::sc_signal<sc_dt::sc_uint<8>> aluOperand;
    sc_core::sc_signal<sc_dt::sc_uint<4>> aluOpcode;
    sc_core::sc_signal<sc_dt::sc_uint<8>> aluResult;
    sc_core::sc_signal<sc_dt::sc_uint<5>> aluFlags;

    ControlUnitConsumer() {
        cu.clock(clock);

        // Bind signals to ControlUnit ports
        cu.memDataOut(memDataOut);
        cu.memDataIn(memDataIn);
        cu.memAddress(memAddress);
        cu.memRead(memRead);
        cu.memWrite(memWrite);
        
        // Bind ALU signals to ControlUnit
        cu.aluAccumulator(aluAccumulator);
        cu.aluOperand(aluOperand);
        cu.aluOpcode(aluOpcode);
        cu.aluResult(aluResult);
        cu.aluFlags(aluFlags);

        // Bind signals to Memory
        memory.dataOut(memDataOut);
        memory.dataIn(memDataIn);
        memory.address(memAddress);
        memory.read(memRead);
        memory.write(memWrite);
        
        // Bind signals to ALU
        alu.accumulator(aluAccumulator);
        alu.operand(aluOperand);
        alu.opcode(aluOpcode);
        alu.result(aluResult);
        alu.flags(aluFlags);
    }

    void loadMemory(const std::array<uint8_t, MemorySize>& data) {
        memory.load(data);
    }

    void inspect() {
        cu.inspect();
    }
};

// We need to create all modules and set all signals before starting any simulations.
static modules::add<ControlUnitConsumer> gControlUnit;

#pragma mark - Control Unit Tests

TEST(ControlUnitTests, ADIInstructionTest) {
    auto cu = modules::get<ControlUnitConsumer>();
    
    std::array<uint8_t, MemorySize> program = {
        0b11000110, 0b00000101, // ADI 5
        0b01110110,             // HLT
    };
    cu->loadMemory(program);

    sc_start();

    // TODO: Verify that the result is as expected
    cu->inspect();
}
//
//  processor.hpp
//
//  Created by Ilia Shoshin on 07.10.24.
//

#pragma once

#include "alu.hpp"
#include "reg.hpp"
#include "memory.hpp"
#include "mut.hpp"
#include "cu.hpp"

#include <systemc>

namespace sim {

class Intel8080 final : public sc_core::sc_module {

    // Configure clock to have 0.5 microsecond period (2 MHz)
    sc_core::sc_clock clock {"clock", 0.5, sc_core::SC_US};

public:
    ALU alu {"ALU"};
    ControlUnit cu {"ControlUnit"};
    Multiplexer mux {"MUX"};
    Memory<DEFAULT_MEMORY_SIZE> memory {"Memory"};
    Register registerA {"registerA"};
    Register registerB {"registerB"};
    Register registerC {"registerC"};
    Register registerD {"registerD"};
    Register registerE {"registerE"};
    Register registerH {"registerH"};
    Register registerL {"registerL"};

    Intel8080(sc_core::sc_module_name name) 
        : sc_core::sc_module(std::move(name)) {
         // Registers signal connections
        registerA.clock(clock);
        registerA.writeEnable(aWriteEnable);
        registerA.readEnable(aReadEnable);
        registerA.dataIn(inputA);
        registerA.dataOut(outputA);

        registerB.clock(clock);
        registerB.writeEnable(bWriteEnable);
        registerB.readEnable(bReadEnable);
        registerB.dataIn(inputB);
        registerB.dataOut(outputB);

        registerC.clock(clock);
        registerC.writeEnable(cWriteEnable);
        registerC.readEnable(cReadEnable);
        registerC.dataIn(inputC);
        registerC.dataOut(outputC);

        registerD.clock(clock);
        registerD.writeEnable(dWriteEnable);
        registerD.readEnable(dReadEnable);
        registerD.dataIn(inputD);
        registerD.dataOut(outputD);

        registerE.clock(clock);
        registerE.writeEnable(eWriteEnable);
        registerE.readEnable(eReadEnable);
        registerE.dataIn(inputE);
        registerE.dataOut(outputE);

        registerH.clock(clock);
        registerH.writeEnable(hWriteEnable);
        registerH.readEnable(hReadEnable);
        registerH.dataIn(inputH);
        registerH.dataOut(outputH);

        registerL.clock(clock);
        registerL.writeEnable(lWriteEnable);
        registerL.readEnable(lReadEnable);
        registerL.dataIn(inputL);
        registerL.dataOut(outputL);

        // Memory signal connections
        memory.addressBus(addressBus);
        memory.writeEnable(memoryWriteEnable);
        memory.readEnable(memoryReadEnable);
        memory.dataBusIn(dataBusControlUnitMemory);
        memory.dataBusOut(dataBusMemoryControlUnit);

        // Connect MUX signals
        mux.inputB(outputB);
        mux.inputC(outputC);
        mux.inputD(outputD);
        mux.inputE(outputE);
        mux.inputH(outputH);
        mux.inputL(outputL);
        mux.inputA(outputA);
        mux.outputB(inputB);
        mux.outputC(inputC);
        mux.outputD(inputD);
        mux.outputE(inputE);
        mux.outputH(inputH);
        mux.outputL(inputL);
        mux.outputA(inputA);
        
        mux.regWriteEnable[SELECT_REG_A](aWriteEnable);
        mux.regWriteEnable[SELECT_REG_B](bWriteEnable);
        mux.regWriteEnable[SELECT_REG_C](cWriteEnable);
        mux.regWriteEnable[SELECT_REG_D](dWriteEnable);
        mux.regWriteEnable[SELECT_REG_E](eWriteEnable);
        mux.regWriteEnable[SELECT_REG_H](hWriteEnable);
        mux.regWriteEnable[SELECT_REG_L](lWriteEnable);

        mux.regReadEnable[SELECT_REG_A](aReadEnable);
        mux.regReadEnable[SELECT_REG_B](bReadEnable);
        mux.regReadEnable[SELECT_REG_C](cReadEnable);
        mux.regReadEnable[SELECT_REG_D](dReadEnable);
        mux.regReadEnable[SELECT_REG_E](eReadEnable);
        mux.regReadEnable[SELECT_REG_H](hReadEnable);
        mux.regReadEnable[SELECT_REG_L](lReadEnable);

        mux.select(muxSelect);
        mux.input(dataControlUnitMux);
        mux.output(dataMuxControlUnit);
        mux.writeEnable(muxWriteEnable);
        mux.readEnable(muxReadEnable);

        // Control unit signal connections
        cu.clock(clock);                                // Connect clock to the Control Unit
        cu.dataBusOut(dataBusControlUnitMemory);
        cu.dataBusIn(dataBusMemoryControlUnit);
        cu.addressBus(addressBus);
        cu.muxSelect(muxSelect);                        // Control Unit selects MUX output
        cu.inputMux(dataMuxControlUnit);
        cu.outputMux(dataControlUnitMux);
        cu.memoryReadEnable(memoryReadEnable);          // Control Unit manages read
        cu.memoryWriteEnable(memoryWriteEnable);        // Control Unit manages write
        cu.muxReadEnable(muxReadEnable);
        cu.muxWriteEnable(muxWriteEnable);

        // ALU signal connections

        alu.accumulator(aluAccumulator);
        alu.operand(aluOperand);
        alu.opcode(aluOpCode);
        alu.result(aluResult);
        alu.flags(aluFlags);

        cu.aluAccumulator(aluAccumulator);     // Control Unit output to ALU input A
        cu.aluOperand(aluOperand);             // Control Unit output to ALU input Arg
        cu.aluOpcode(aluOpCode);               // Control Unit output to ALU opcode
        cu.aluResult(aluResult);               // ALU result to Control Unit input
        cu.aluFlags(aluFlags);                 // ALU flags to Control Unit input
    }

    void loadMemory(const std::array<uint8_t, DEFAULT_MEMORY_SIZE>& data) {
        memory.load(data);
    }

private:
    // Data Lines
    sc_core::sc_signal<sc_dt::sc_uint<8>> inputA, inputB, inputC, inputD, inputE, inputH, inputL;
    sc_core::sc_signal<sc_dt::sc_uint<8>> outputA, outputB, outputC, outputD, outputE, outputH, outputL;
    sc_core::sc_signal<sc_dt::sc_uint<8>> dataControlUnitMux;
    sc_core::sc_signal<sc_dt::sc_uint<8>> dataMuxControlUnit;
    sc_core::sc_signal<sc_dt::sc_uint<8>> dataBusControlUnitMemory;
    sc_core::sc_signal<sc_dt::sc_uint<8>> dataBusMemoryControlUnit;
    // Address Lines
    sc_core::sc_signal<sc_dt::sc_uint<16>> addressBus;
    // Control Lines
    sc_core::sc_signal<sc_dt::sc_uint<8>> muxSelect;

    sc_core::sc_signal<bool> memoryWriteEnable;
    sc_core::sc_signal<bool> aWriteEnable;
    sc_core::sc_signal<bool> bWriteEnable;
    sc_core::sc_signal<bool> cWriteEnable;
    sc_core::sc_signal<bool> dWriteEnable;
    sc_core::sc_signal<bool> eWriteEnable;
    sc_core::sc_signal<bool> hWriteEnable;
    sc_core::sc_signal<bool> lWriteEnable;
    sc_core::sc_signal<bool> muxWriteEnable;

    sc_core::sc_signal<bool> memoryReadEnable;
    sc_core::sc_signal<bool> aReadEnable;
    sc_core::sc_signal<bool> bReadEnable;
    sc_core::sc_signal<bool> cReadEnable;
    sc_core::sc_signal<bool> dReadEnable;
    sc_core::sc_signal<bool> eReadEnable;
    sc_core::sc_signal<bool> hReadEnable;
    sc_core::sc_signal<bool> lReadEnable;
    sc_core::sc_signal<bool> muxReadEnable;

    // ALU Lines
    sc_core::sc_signal<sc_dt::sc_uint<8>> aluAccumulator;// ALU input A signal
    sc_core::sc_signal<sc_dt::sc_uint<8>> aluOperand;    // ALU input Arg signal
    sc_core::sc_signal<sc_dt::sc_uint<4>> aluOpCode;     // ALU OpCode signal
    sc_core::sc_signal<sc_dt::sc_uint<8>> aluResult;     // ALU result signal
    sc_core::sc_signal<sc_dt::sc_uint<5>> aluFlags;      // ALU flags signal
};

}

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
        registerA.writeEnable(aWriteEnable);
        registerA.dataIn(dataMuxToRegA);
        registerA.dataOut(dataRegAToMux);

        registerB.writeEnable(bWriteEnable);
        registerB.dataIn(dataMuxToRegB);
        registerB.dataOut(dataRegBToMux);

        registerC.writeEnable(cWriteEnable);
        registerC.dataIn(dataMuxToRegC);
        registerC.dataOut(dataRegCToMux);

        registerD.writeEnable(dWriteEnable);
        registerD.dataIn(dataMuxToRegD);
        registerD.dataOut(dataRegDToMux);

        registerE.writeEnable(eWriteEnable);
        registerE.dataIn(dataMuxToRegE);
        registerE.dataOut(dataRegEToMux);

        registerH.writeEnable(hWriteEnable);
        registerH.dataIn(dataMuxToRegH);
        registerH.dataOut(dataRegHToMux);

        registerL.writeEnable(lWriteEnable);
        registerL.dataIn(dataMuxToRegL);
        registerL.dataOut(dataRegLToMux);

        // Memory signal connections
        memory.addressBus(addressBus);
        memory.writeEnable(memoryWriteEnable);
        memory.readEnable(memoryReadEnable);
        memory.dataBusIn(dataBusControlUnitMemory);
        memory.dataBusOut(dataBusMemoryControlUnit);

        // Connect MUX signals
        mux.inputA(dataRegAToMux);
        mux.inputB(dataRegBToMux);
        mux.inputC(dataRegCToMux);
        mux.inputD(dataRegDToMux);
        mux.inputE(dataRegEToMux);
        mux.inputH(dataRegHToMux);
        mux.inputL(dataRegLToMux);
        mux.outputA(dataMuxToRegA);
        mux.outputB(dataMuxToRegB);
        mux.outputC(dataMuxToRegC);
        mux.outputD(dataMuxToRegD);
        mux.outputE(dataMuxToRegE);
        mux.outputH(dataMuxToRegH);
        mux.outputL(dataMuxToRegL);
        
        mux.regWriteEnable[SELECT_REG_A](aWriteEnable);
        mux.regWriteEnable[SELECT_REG_B](bWriteEnable);
        mux.regWriteEnable[SELECT_REG_C](cWriteEnable);
        mux.regWriteEnable[SELECT_REG_D](dWriteEnable);
        mux.regWriteEnable[SELECT_REG_E](eWriteEnable);
        mux.regWriteEnable[SELECT_REG_H](hWriteEnable);
        mux.regWriteEnable[SELECT_REG_L](lWriteEnable);

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

    void reset() {
        memory.reset();
        registerA.reset();
        registerB.reset();
        registerC.reset();
        registerD.reset();
        registerE.reset();
        registerH.reset();
        registerL.reset();
        cu.reset();
    }

    void loadMemory(const std::array<uint8_t, DEFAULT_MEMORY_SIZE>& data) {
#ifdef ENABLE_TESTING
        reset();
        memory.load(data);
        cu.resetHalted();
#else
        memory.load(data);
#endif
    }

private:
    // Data Lines
    sc_core::sc_signal<sc_dt::sc_uint<8>> dataMuxToRegA, dataMuxToRegB, dataMuxToRegC, dataMuxToRegD, dataMuxToRegE, dataMuxToRegH, dataMuxToRegL;
    sc_core::sc_signal<sc_dt::sc_uint<8>> dataRegAToMux, dataRegBToMux, dataRegCToMux, dataRegDToMux, dataRegEToMux, dataRegHToMux, dataRegLToMux;
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
    sc_core::sc_signal<bool> muxReadEnable;

    // ALU Lines
    sc_core::sc_signal<sc_dt::sc_uint<8>> aluAccumulator;// ALU input A signal
    sc_core::sc_signal<sc_dt::sc_uint<8>> aluOperand;    // ALU input Arg signal
    sc_core::sc_signal<sc_dt::sc_uint<4>> aluOpCode;     // ALU OpCode signal
    sc_core::sc_signal<sc_dt::sc_uint<8>> aluResult;     // ALU result signal
    sc_core::sc_signal<sc_dt::sc_uint<5>> aluFlags;      // ALU flags signal
};

}

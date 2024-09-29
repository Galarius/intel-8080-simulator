#include "alu.hpp"
#include "memory.hpp"
#include "cu.hpp"
#include "log.hpp"

#include <systemc>

using namespace sim;
using namespace sc_core;

namespace {
    auto logger() { return spdlog::get(LogName::main); }
}

int sc_main(__unused int argc, __unused char* argv[]) 
{
    ConfigureFileLogging("simulator.log", spdlog::level::trace);

    ALU alu("ALU");
    Memory<DEFAULT_MEMORY_SIZE> memory("Memory");
    ControlUnit cu("ControlUnit");
    sc_clock clock("clock", 0.5, SC_US);        // Configure clock to have 0.5 microsecond period (2 MHz)

    // Signals

    sc_signal<sc_dt::sc_uint<8>> aluA;          // ALU input A signal
    sc_signal<sc_dt::sc_uint<8>> aluArg;        // ALU input Arg signal
    sc_signal<sc_dt::sc_uint<4>> aluOpCode;     // ALU OpCode signal
    sc_signal<sc_dt::sc_uint<8>> aluResult;     // ALU result signal
    sc_signal<sc_dt::sc_uint<5>> aluFlags;      // ALU flags signal

    sc_signal<sc_dt::sc_uint<16>> memAddress;   // Memory address signal
    sc_signal<sc_dt::sc_uint<8>>  memDataIn;    // Memory data in signal
    sc_signal<sc_dt::sc_uint<8>>  memDataOut;   // Memory data out signal
    sc_signal<bool> memRead;                    // Memory read signal
    sc_signal<bool> memWrite;                   // Memory write signal


    // ALU signal connections

    alu.inA(aluA);
    alu.inArg(aluArg);
    alu.inOpCode(aluOpCode);
    alu.outResult(aluResult);
    alu.outFlags(aluFlags);

    cu.aluOutA(aluA);               // Control Unit output to ALU input A
    cu.aluOutArg(aluArg);           // Control Unit output to ALU input Arg
    cu.aluOutOpCode(aluOpCode);     // Control Unit output to ALU opcode
    cu.aluInResult(aluResult);      // ALU result to Control Unit input
    cu.aluInFlags(aluFlags);        // ALU flags to Control Unit input

    memory.address(memAddress);
    memory.dataIn(memDataIn);
    memory.dataOut(memDataOut);
    memory.read(memRead);
    memory.write(memWrite);

    cu.clock(clock);
    cu.memOutAddress(memAddress);   // Control Unit to Memory address
    cu.memOutDataIn(memDataIn);     // Control Unit to Memory data in
    cu.memOutRead(memRead);         // Control Unit read signal to Memory
    cu.memOutWrite(memWrite);       // Control Unit write signal to Memory
    cu.memInDataOut(memDataOut);    // Memory data output to Control Unit

    sc_start();

    logger()->info("Shutting down...\n\n");
    spdlog::shutdown();

    return 0;
}

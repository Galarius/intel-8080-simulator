#include "alu.hpp"
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

    ALU alu("ALU"); // Arithmetic and Logic Unit

    // TODO: Remove temporary signals
    sc_core::sc_signal<sc_dt::sc_uint<8>> sigA;
    sc_core::sc_signal<sc_dt::sc_uint<8>> sigArg;
    sc_core::sc_signal<sc_dt::sc_uint<4>> sigOpCode;
    sc_core::sc_signal<sc_dt::sc_uint<8>> sigResult;
    sc_core::sc_signal<sc_dt::sc_uint<5>> sigFlags;
    //

    alu.inA(sigA);
    alu.inArg(sigArg);
    alu.inOpCode(sigOpCode);
    alu.outResult(sigResult);
    alu.outFlags(sigFlags);
    
    sc_start();

    logger()->info("Shutting down...\n\n");
    spdlog::shutdown();
    return 0;
}

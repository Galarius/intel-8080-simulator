#include "processor.hpp"
#include "log.hpp"

#include <systemc>

using namespace sim;

namespace {
    auto logger() { return spdlog::get(LogName::main); }
}

int sc_main(__unused int argc, __unused char* argv[]) {
    ConfigureFileLogging("simulator.log", spdlog::level::trace);

    Intel8080 processor("Intel8080");
    std::array<uint8_t, DEFAULT_MEMORY_SIZE> program = {
        0b00000110, 18,  // MVI B, 18
        0b00001110, 19,  // MVI C, 19
        0b00010110, 20,  // MVI D, 20
        0b00011110, 21,  // MVI E, 21
        0b00100110, 22,  // MVI H, 22
        0b00101110, 23,  // MVI L, 23
        0b00111110, 24,  // MVI A, 24
        0b01110110       // HLT
    };
    processor.loadMemory(program);

    sc_core::sc_start();

    logger()->info("Shutting down...\n\n");
    spdlog::shutdown();

    return 0;
}

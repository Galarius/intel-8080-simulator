//
//  cu.cpp
//
//  Created by Ilia Shoshin on 29.09.24.
//

#include "cu.hpp"
#include "log.hpp"

#include <systemc>

using namespace sc_core;

// namespace {
//     auto logger() { return spdlog::get(sim::LogName::cu); }
// }

namespace sim {

ControlUnit::ControlUnit(sc_core::sc_module_name name)
    : sc_core::sc_module(name), pc(0) {
    SC_THREAD(execute);

    sensitive << clock.pos();  // Add clock sensitivity for positive edge
}

void ControlUnit::execute() {
    while (true) {
        // logger()->trace("cthread triggered @ {}", sc_time_stamp().to_string());
        fetchInstruction();
        decodeInstruction();
        executeInstruction();
        wait();
    }
}

void ControlUnit::fetchInstruction() {
    memAddress.write(pc);    // Set memory address to fetch instruction
    memRead.write(true);     // Set read signal high
    
    wait();                     // Wait for one cycle
    memRead.write(false);    // Clear read signal
    // Read instruction from memory
    sc_dt::sc_uint<8> instruction = memDataOut.read();
    // logger()->trace("Fetched instruction {}", instruction.to_uint());
    ++pc;
}

void ControlUnit::decodeInstruction() {
    // TODO
}

void ControlUnit::executeInstruction() {
    // TODO
}

} // namespace sim

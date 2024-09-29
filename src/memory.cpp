//
//  memory.hpp
//
//  Created by Ilia Shoshin on 28.09.24.
//

#include "memory.hpp"
#include "log.hpp"

using namespace sc_core;

namespace {
    auto logger() { return spdlog::get(sim::LogName::memory); }
}

namespace sim {

Memory::Memory(sc_module_name name)
    : sc_module(std::move(name)), mem {} {
    SC_METHOD(execute);
    // Process on read/write signals and address change
    sensitive << read << write << address << dataIn;
}

void Memory::execute() {
    if (write.read()) {
        // Write data to memory
        mem[address.read().to_uint()] = dataIn.read();
        logger()->info("Written to memory: Address={}, Data={}", address.read().to_int(), dataIn.read().to_uint());
    }

    if (read.read()) {
        // Read data from memory
        dataOut.write(mem[address.read().to_uint()]);
        logger()->info("Read from memory: Address={}, Data={}", address.read().to_int(), dataOut.read().to_uint());
    }
}

void Memory::load(const std::array<uint8_t, Memory::size>& data) {
    std::copy(data.begin(), data.end(), mem.begin());
}

} // namespace sim
//
//  memory.tpp
//
//  Created by Ilia Shoshin on 28.09.24.
//

#pragma once

#include <systemc>
#include "memory.hpp"
#include "log.hpp"

namespace sim {

template<size_t MemorySize>
Memory<MemorySize>::Memory(sc_core::sc_module_name name)
    : sc_module(std::move(name)), buffer {} {
    SC_METHOD(execute);
    // Process on read/write signals and address change
    sensitive << read << write << address << dataIn;
}

template<size_t MemorySize>
void Memory<MemorySize>::execute() {
    if (write.read()) {
        // Write data to memory
        buffer[address.read().to_uint() % MemorySize] = dataIn.read();
        spdlog::get(sim::LogName::memory)->info("Written to memory: Address={}, Data={}", address.read().to_int(), dataIn.read().to_uint());
    }

    if (read.read()) {
        // Read data from memory
        dataOut.write(buffer[address.read().to_uint() % MemorySize]);
        spdlog::get(sim::LogName::memory)->info("Read from memory: Address={}, Data={}", address.read().to_int(), dataOut.read().to_uint());
    }
}

template<size_t MemorySize>
void Memory<MemorySize>::load(const std::array<uint8_t, MemorySize>& data) {
    std::copy(data.begin(), data.end(), buffer.begin());
}

} // namespace sim
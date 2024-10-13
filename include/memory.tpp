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
    sensitive << readEnable << writeEnable << addressBus << dataBusIn;
    dont_initialize();
}

template<size_t MemorySize>
void Memory<MemorySize>::reset() {
    buffer.fill(0);
}

template<size_t MemorySize>
void Memory<MemorySize>::execute() {
    if (writeEnable.read()) {
        // Write data to memory
        buffer[addressBus.read().to_uint() % MemorySize] = dataBusIn.read();
        spdlog::get(sim::LogName::memory)->info("Written to memory: Address={}, Data={}", addressBus.read().to_int(), dataBusIn.read().to_uint());
    }

    if (readEnable.read()) {
        // Read data from memory
        dataBusOut.write(buffer[addressBus.read().to_uint() % MemorySize]);
        spdlog::get(sim::LogName::memory)->info("Read from memory: Address={}, Data={}", addressBus.read().to_int(), dataBusOut.read().to_uint());
    }
}

template<size_t MemorySize>
void Memory<MemorySize>::load(const std::array<uint8_t, MemorySize>& data) {
    spdlog::get(sim::LogName::memory)->info("Loading program...");
    std::copy(data.begin(), data.end(), buffer.begin());
}

} // namespace sim

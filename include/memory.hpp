//
//  memory.hpp
//
//  Created by Ilia Shoshin on 28.09.24.
//
#pragma once

#include <stdexcept>
#include <systemc>
#include <array>

namespace sim {

constexpr uint32_t DEFAULT_MEMORY_SIZE = 65536; // 64 KB

template<size_t MemorySize>
class Memory final : public sc_core::sc_module {
public:
    // Ports
    sc_core::sc_in<sc_dt::sc_uint<16>> address;     // 16-bit address input
    sc_core::sc_in<sc_dt::sc_uint<8>>  dataIn;      // Data input for writing
    sc_core::sc_out<sc_dt::sc_uint<8>> dataOut;     // Data output for reading
    sc_core::sc_in<bool> read;                      // Read signal
    sc_core::sc_in<bool> write;                     // Write signal

    Memory(sc_core::sc_module_name name);

    void load(const std::array<uint8_t, MemorySize>& data);

private:
    void execute();

    std::array<sc_dt::sc_uint<8>, MemorySize> buffer;   // Internal memory storage
};

} // namespace sim

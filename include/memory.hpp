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
    sc_core::sc_in<sc_dt::sc_uint<16>> addressBus;
    sc_core::sc_in<sc_dt::sc_uint<8>>  dataBusIn;
    sc_core::sc_out<sc_dt::sc_uint<8>> dataBusOut;
    sc_core::sc_in<bool> readEnable;
    sc_core::sc_in<bool> writeEnable;

    Memory(sc_core::sc_module_name name);

    void load(const std::array<uint8_t, MemorySize>& data);

private:
    void execute();

    std::array<sc_dt::sc_uint<8>, MemorySize> buffer;   // Internal memory storage
};

} // namespace sim

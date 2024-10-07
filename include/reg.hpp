//
//  reg.hpp
//
//  Created by Ilia Shoshin on 01.10.24.
//

#pragma once

#include <systemc>

namespace sim {

class Register : public sc_core::sc_module {
public:
    sc_core::sc_in<bool> clock; // Using clocked register to ensure that all updates happen synchronously with the CPU cycles, matching the timing of real hardware.
    sc_core::sc_in<bool> writeEnable;  // Write enable signal
    sc_core::sc_in<bool> readEnable;   // Read enable signal
    sc_core::sc_in<sc_dt::sc_uint<8>> dataIn;  // Data to be written to the register
    sc_core::sc_out<sc_dt::sc_uint<8>> dataOut;  // Data read from the register

    Register(sc_core::sc_module_name);

private:
    void update();

    sc_dt::sc_uint<8> value;
};

} // namespace sim

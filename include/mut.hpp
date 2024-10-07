//
//  mut.hpp
//
//  Created by Ilia Shoshin on 01.10.24.
//

#pragma once

#include "common.hpp"

#include <systemc>

namespace sim {

class Multiplexer : public sc_core::sc_module {

public:

    sc_core::sc_in<sc_dt::sc_uint<8>>  select;      // Active source
    sc_core::sc_in<sc_dt::sc_uint<8>>  input;       // Input signal
    sc_core::sc_out<sc_dt::sc_uint<8>> output;      // Output signal
    sc_core::sc_in<bool>               writeEnable; // Control signal to enable writing
    sc_core::sc_in<bool>               readEnable;  // Control signal to enable reading

    // Input ports for each register and memory
    sc_core::sc_in<sc_dt::sc_uint<8>> inputA;
    sc_core::sc_in<sc_dt::sc_uint<8>> inputB;
    sc_core::sc_in<sc_dt::sc_uint<8>> inputC;
    sc_core::sc_in<sc_dt::sc_uint<8>> inputD;
    sc_core::sc_in<sc_dt::sc_uint<8>> inputE;
    sc_core::sc_in<sc_dt::sc_uint<8>> inputH;
    sc_core::sc_in<sc_dt::sc_uint<8>> inputL;

    // Output ports for each register and memory
    sc_core::sc_out<sc_dt::sc_uint<8>> outputA;
    sc_core::sc_out<sc_dt::sc_uint<8>> outputB;
    sc_core::sc_out<sc_dt::sc_uint<8>> outputC;
    sc_core::sc_out<sc_dt::sc_uint<8>> outputD;
    sc_core::sc_out<sc_dt::sc_uint<8>> outputE;
    sc_core::sc_out<sc_dt::sc_uint<8>> outputH;
    sc_core::sc_out<sc_dt::sc_uint<8>> outputL;

    sc_core::sc_out<bool> regWriteEnable[7];
    sc_core::sc_out<bool> regReadEnable[7];

    Multiplexer(sc_core::sc_module_name name) : sc_core::sc_module(name) {
        SC_METHOD(selector);
        sensitive << select << input << writeEnable << readEnable;
    }

private:
    void selector() {
        if (writeEnable.read()) {
            // Write to the selected source
            const uint regID = select.read().to_uint();
            regWriteEnable[regID].write(true);
            switch (regID) {
                case SELECT_REG_A: outputA.write(input.read()); break;
                case SELECT_REG_B: outputB.write(input.read()); break;
                case SELECT_REG_C: outputC.write(input.read()); break;
                case SELECT_REG_D: outputD.write(input.read()); break;
                case SELECT_REG_E: outputE.write(input.read()); break;
                case SELECT_REG_H: outputH.write(input.read()); break;
                case SELECT_REG_L: outputL.write(input.read()); break;
                default: break;
            }
        }

        if (readEnable.read()) {
            // Read from the selected source
            const uint regID = select.read().to_uint();
            regReadEnable[regID].write(true);
            switch (regID) {
                case SELECT_REG_A: output.write(inputA.read()); break;
                case SELECT_REG_B: output.write(inputB.read()); break;
                case SELECT_REG_C: output.write(inputC.read()); break;
                case SELECT_REG_D: output.write(inputD.read()); break;
                case SELECT_REG_E: output.write(inputE.read()); break;
                case SELECT_REG_H: output.write(inputH.read()); break;
                case SELECT_REG_L: output.write(inputL.read()); break;
                default: break;
            }
        }
    }
};

} // namespace sim

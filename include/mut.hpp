//
//  mut.hpp
//
//  Created by Ilia Shoshin on 01.10.24.
//

#pragma once

#include "common.hpp"
#include "log.hpp"
#include "utils.hpp"

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

    Multiplexer(sc_core::sc_module_name name) : sc_core::sc_module(name) {
        SC_METHOD(selector);
        sensitive << select << writeEnable << readEnable << input << inputA << inputB << inputC << inputD << inputE << inputH << inputL;
        dont_initialize();
    }

private:
    void selector() {
        const uint regID = select.read().to_uint();
        spdlog::get(sim::LogName::mut)->trace("Selector -> {} ", utils::to_binary(regID));
        if (writeEnable.read()) {
            spdlog::get(sim::LogName::mut)->trace("Writing register {} ", utils::to_binary(regID));
            // Write to the selected source
            regWriteEnable[regID].write(true);
            const sc_dt::sc_uint<8> data = input.read();
            switch (regID) {
                case SELECT_REG_A: outputA.write(data); break;
                case SELECT_REG_B: outputB.write(data); break;
                case SELECT_REG_C: outputC.write(data); break;
                case SELECT_REG_D: outputD.write(data); break;
                case SELECT_REG_E: outputE.write(data); break;
                case SELECT_REG_H: outputH.write(data); break;
                case SELECT_REG_L: outputL.write(data); break;
                default: break;
            }
        } else {
            regWriteEnable[regID].write(false);
        }

        if (readEnable.read()) {
            spdlog::get(sim::LogName::mut)->trace("Reading register {} ", utils::to_binary(regID));
            // Read from the selected source
            sc_dt::sc_uint<8> data;
            switch (regID) {
                case SELECT_REG_A: data = inputA.read(); break;
                case SELECT_REG_B: data = inputB.read(); break;
                case SELECT_REG_C: data = inputC.read(); break;
                case SELECT_REG_D: data = inputD.read(); break;
                case SELECT_REG_E: data = inputE.read(); break;
                case SELECT_REG_H: data = inputH.read(); break;
                case SELECT_REG_L: data = inputL.read(); break;
                default: break;
            }
            output.write(data);
        }
    }
};

} // namespace sim

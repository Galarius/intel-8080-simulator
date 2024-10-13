//
//  reg.cpp
//
//  Created by Ilia Shoshin on 01.10.24.
//

#include "reg.hpp"
#include "log.hpp"

#include <systemc>

using namespace sc_core;

namespace sim {

Register::Register(sc_module_name name) : sc_module(name) {
    SC_METHOD(update);
    sensitive << writeEnable << dataIn;
    dont_initialize();
}

void Register::reset() {
    value = 0;
}

void Register::update() {
    if (writeEnable.read()) {
        value = dataIn.read();
        spdlog::get(sim::LogName::reg)->trace("The value {} has been set", value.to_uint());
    }
    dataOut.write(value);
}

} // namespace sim

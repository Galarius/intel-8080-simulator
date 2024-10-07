//
//  reg.cpp
//
//  Created by Ilia Shoshin on 01.10.24.
//

#include "reg.hpp"

#include <systemc>

using namespace sc_core;

namespace sim {

Register::Register(sc_module_name name) : sc_module(name) {
    SC_METHOD(update);
    sensitive << clock.pos();
}

void Register::update() {
    if (writeEnable.read()) {
        value = dataIn.read();
    }
    if(readEnable.read()) {
        dataOut.write(value);
    }
}

} // namespace sim

//
//  cu.cpp
//
//  Created by Ilia Shoshin on 29.09.24.
//

#include "cu.hpp"
#include "log.hpp"
#include "utils.hpp"

#include <systemc>

using namespace sc_core;

namespace {
    auto logger() { return spdlog::get(sim::LogName::cu); }
}

namespace sim {

const uint8_t ControlUnit::OP_REG_B = 0b00000000;
const uint8_t ControlUnit::OP_REG_C = 0b00000001;
const uint8_t ControlUnit::OP_REG_D = 0b00000010;
const uint8_t ControlUnit::OP_REG_E = 0b00000011;
const uint8_t ControlUnit::OP_REG_H = 0b00000100;
const uint8_t ControlUnit::OP_REG_L = 0b00000101;
const uint8_t ControlUnit::OP_REG_M = 0b00000110;    // Memory, refers to the address in the HL register pair)
const uint8_t ControlUnit::OP_REG_A = 0b00000111;    // Accumulator

const uint8_t ControlUnit::OP_GROUP_DATA_TRANSFER = 0b00000000;
const uint8_t ControlUnit::OP_GROUP_MOV = 0b00000001;
const uint8_t ControlUnit::OP_GROUP_ALU = 0b00000010;
const uint8_t ControlUnit::OP_GROUP_SPECIAL = 0b00000011;

const uint8_t ControlUnit::OP_INST_NOP = 0b00000000;
const uint8_t ControlUnit::OP_INST_HLT = 0b01110110;

ControlUnit::ControlUnit(sc_core::sc_module_name name)
    : sc_core::sc_module(name), pc(0) {
    SC_THREAD(execute);

    sensitive << clock.pos();  // Add clock sensitivity for positive edge
}

void ControlUnit::signalToReadAt(sc_dt::sc_uint<16> address) {
    memAddress.write(address);  // Set memory address to fetch instruction/operand
    memRead.write(true);        // Set read signal high
    wait(SC_ZERO_TIME);         // Wait for one cycle
    memRead.write(false);       // Clear read signal
}

void ControlUnit::waitFor(int clocks) {
    for (int i = 0; i < clocks; ++i) {
        logger()->trace("wait for 1 cycle");
        wait(SC_ZERO_TIME); // 1 cycle
    }
}

void ControlUnit::execute() {
    while (true) {
        logger()->trace("cthread triggered @ {}", sc_time_stamp().to_string());
        // fetch & decode & execute
        signalToReadAt(pc); // 1 cycle
        const uint8_t instruction = memDataOut.read().to_uint();
        const uint8_t opgroup = (instruction >> 6) & 0b00000011;
        const uint8_t opcode = (instruction >> 3) & 0b00000111;
        const uint8_t source = instruction & 0b00000111;

        if(instruction != OP_INST_NOP) {
            logger()->trace("pc -> {} (group: {}, code: {}, source: {})", 
                utils::to_binary(instruction), 
                utils::to_binary(opgroup), 
                utils::to_binary(opcode), 
                utils::to_binary(source));
        }

        switch (opgroup)
        {
        case OP_GROUP_DATA_TRANSFER:
            if(instruction == OP_INST_NOP) { // NOP
                waitFor(4);
                ++pc;
            }
        break;
    
        case OP_GROUP_MOV:
            if(instruction == OP_INST_HLT) { // HLT
                waitFor(7);
                logger()->info("HLT: Stopping execution...");
                sc_stop();
            }
        break;

        case OP_GROUP_ALU:
            aluOpcode.write(opcode);
            aluAccumulator.write(accumulator);
            aluOperand.write(getRegisterValue(source)); // 1 cycle if source is OP_REG_M
            if(source == OP_REG_M) { // If the operand is located in memory -> 7 clocks cycles - 2 cycles that have already passed
                waitFor(5);
            } else {
                waitFor(3);
            }
            accumulator = aluResult.read();
            flags = aluFlags.read();
            ++pc;
            break;
        
        case OP_GROUP_SPECIAL:
            if(source == OP_REG_M) { // ALU Immediate
                signalToReadAt(++pc); // 1 cycle
                aluOpcode.write(opcode);
                aluAccumulator.write(accumulator);
                aluOperand.write(memDataOut.read());
                waitFor(5); // clocks = 7 - 2
                accumulator = aluResult.read();
                flags = aluFlags.read();
                ++pc;
            }
            break;

        default:
            std::cerr << "Unknown opcode: " << std::hex << (int)instruction << std::dec << std::endl;
            break;
        }
                
        wait();
    }
}

void ControlUnit::inspect() {
    logger()->debug("A: {}, B: {}. C: {}, D: {}, E: {}, H: {}, L: {}, SP: {}, PC: {}, Flags: Z {}; C {}; S {}; P {}; AC {}.",
    accumulator.to_uint(), 
    registerB.to_uint(),
    registerC.to_uint(),
    registerD.to_uint(),
    registerE.to_uint(),
    registerH.to_uint(),
    registerL.to_uint(),
    sp.to_uint(),
    pc.to_uint(),
    flags[0].to_bool(), 
    flags[1].to_bool(), 
    flags[2].to_bool(), 
    flags[3].to_bool(), 
    flags[4].to_bool());
}

// Decode function to get the source register
sc_dt::sc_uint<8> ControlUnit::getRegisterValue(uint8_t regCode) {
    switch (regCode) {
        case OP_REG_A: return accumulator;
        case OP_REG_B: return registerB;
        case OP_REG_C: return registerC;
        case OP_REG_D: return registerD;
        case OP_REG_E: return registerE;
        case OP_REG_H: return registerH;
        case OP_REG_L: return registerL;
        case OP_REG_M: 
        {
            // Get the memory value pointed to by HL
            sc_dt::sc_uint<16> address = (registerH << 8) | registerL;
            signalToReadAt(address);  // 1 cycle
            return memDataOut.read(); // Memory at (HL)
        }
        default: return 0; // Should not happen
    }
}

} // namespace sim

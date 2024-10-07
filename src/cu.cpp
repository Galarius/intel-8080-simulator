//
//  cu.cpp
//
//  Created by Ilia Shoshin on 29.09.24.
//

#include "cu.hpp"
#include "log.hpp"
#include "utils.hpp"
#include "common.hpp"

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

const uint8_t ControlUnit::OP_MEM   = 0b00000110;    // Source for MUT that refers to memory

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

sc_dt::sc_uint<8> ControlUnit::readReg(sc_dt::sc_uint<8> source) {
    muxSelect.write(source);        // Set active data source to drive the bus
    muxReadEnable.write(true);      // Set read signal high
    wait(SC_ZERO_TIME);             // Wait for one cycle
    muxReadEnable.write(false);     // Clear read signal
    return inputMux.read();
}

sc_dt::sc_uint<8> ControlUnit::readMemAt(sc_dt::sc_uint<16> address) {
    addressBus.write(address);      // Set memory address to fetch instruction/operand
    memoryReadEnable.write(true);   // Set read signal high
    wait(SC_ZERO_TIME);             // Wait for one cycle
    memoryReadEnable.write(false);  // Clear read signal
    return dataBusIn.read();
}

void ControlUnit::writeReg(sc_dt::sc_uint<8> source, sc_dt::sc_uint<8> value) {
    muxSelect.write(source);        // Set active data source to drive the bus
    outputMux.write(value);
    muxWriteEnable.write(true);     // Set write signal high
    wait(SC_ZERO_TIME);             // Wait for one cycle
    muxWriteEnable.write(false);    // Clear read signal
}

void ControlUnit::writeMemAt(sc_dt::sc_uint<16> address, sc_dt::sc_uint<8> value) {
    addressBus.write(address);      // Set memory address to fetch instruction/operand
    dataBusOut.write(value);
    memoryWriteEnable.write(true);
    wait(SC_ZERO_TIME);             // Wait for one cycle
    memoryWriteEnable.write(false);
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
        const uint8_t instruction = readMemAt(pc).to_uint(); // 1 cycle
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
            aluAccumulator.write(readReg(SELECT_REG_A)); // 1 cycle
            aluOpcode.write(opcode);
            aluOperand.write(getRegisterValue(source)); // 3 cycle if source is OP_REG_M or 1 cycle
            if(source == OP_REG_M) { // If the operand is located in memory -> 7 clocks cycles - 5 cycles that have already passed
                waitFor(2);
            } else {
                waitFor(1);
            }
            writeReg(SELECT_REG_A, aluResult.read());   // 1 cycle
            flags = aluFlags.read();
            ++pc;
            break;
        
        case OP_GROUP_SPECIAL:
            if(source == OP_REG_M) {          // ALU Immediate
                aluOpcode.write(opcode);
                aluOperand.write(readMemAt(++pc));  // 1 cycle
                aluAccumulator.write(readReg(SELECT_REG_A)); // 1 cycle
                waitFor(4); // clocks = 7 - 3
                writeReg(SELECT_REG_A, aluResult.read());
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

// Decode function to get the source register
sc_dt::sc_uint<8> ControlUnit::getRegisterValue(uint8_t regCode) {
    muxSelect.write(regCode);  // Select the source in the multiplexer
    switch (regCode) {
        case OP_REG_A: return readReg(SELECT_REG_A); // 1 cycle
        case OP_REG_B: return readReg(SELECT_REG_B); // 1 cycle            
        case OP_REG_C: return readReg(SELECT_REG_C); // 1 cycle
        case OP_REG_D: return readReg(SELECT_REG_D); // 1 cycle
        case OP_REG_E: return readReg(SELECT_REG_E); // 1 cycle
        case OP_REG_H: return readReg(SELECT_REG_H); // 1 cycle
        case OP_REG_L: return readReg(SELECT_REG_L); // 1 cycle
        case OP_REG_M: 
        {
            const sc_dt::sc_uint<8> h = readReg(SELECT_REG_H); // 1 cycle
            const sc_dt::sc_uint<8> l = readReg(SELECT_REG_L); // 1 cycle
            // Get the memory value pointed to by HL
            const sc_dt::sc_uint<16> address = (h << 8) | l;
            // Memory at (HL), read from the bus
            return readMemAt(address); // 1 cycle
        }
        default: return 0; // Should not happen
    }
}

} // namespace sim

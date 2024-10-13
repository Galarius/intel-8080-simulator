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

const uint8_t ControlUnit::OP_RP_BC = 0b00000000;
const uint8_t ControlUnit::OP_RP_DE = 0b00000001;
const uint8_t ControlUnit::OP_RP_HL = 0b00000010;
const uint8_t ControlUnit::OP_RP_SP = 0b00000011;

const uint8_t ControlUnit::OP_INST_NOP = 0b00000000;
const uint8_t ControlUnit::OP_INST_HLT = 0b01110110;

ControlUnit::ControlUnit(sc_core::sc_module_name name)
    : sc_core::sc_module(name), pc(0) {
    SC_THREAD(execute);
    sensitive << clock.pos();  // Add clock sensitivity for positive edge
    dont_initialize();
}

void ControlUnit::reset() {
    logger()->trace("Resetting...");
    pc = 0x0;
    sp = 0x0;
    flags = 0x0;
#ifdef ENABLE_TESTING
    doResetting();
#endif
}

sc_dt::sc_uint<8> ControlUnit::readReg(sc_dt::sc_uint<8> source) {
    logger()->trace("Reading register {}...", utils::to_binary(source.to_uint()));
    muxSelect.write(source);        // Set active data source
    muxReadEnable.write(true);      // Set read signal high
    wait(SC_ZERO_TIME);             // Wait for one cycle
    muxReadEnable.write(false);     // Clear read signal
    wait(SC_ZERO_TIME);             // Wait for one cycle
    return inputMux.read();
}

sc_dt::sc_uint<8> ControlUnit::readMemAt(sc_dt::sc_uint<16> address) {
    logger()->trace("Reading memory at address {} ", address.to_uint());
    addressBus.write(address);      // Set memory address to fetch instruction/operand
    memoryReadEnable.write(true);   // Set read signal high
    wait(SC_ZERO_TIME);             // Wait for one cycle
    memoryReadEnable.write(false);  // Clear read signal
    wait(SC_ZERO_TIME);             // Wait for one cycle
    return dataBusIn.read();
}

void ControlUnit::writeReg(sc_dt::sc_uint<8> source, sc_dt::sc_uint<8> value) {
    logger()->trace("Writing value {} to register {} ", value.to_uint(), utils::to_binary(source.to_uint()));
    muxSelect.write(source);        // Set active data source to drive the bus
    outputMux.write(value);
    muxWriteEnable.write(true);     // Set write signal high
    wait(SC_ZERO_TIME);             // Wait for one cycle
    muxWriteEnable.write(false);    // Clear read signal
}

void ControlUnit::writeMemAt(sc_dt::sc_uint<16> address, sc_dt::sc_uint<8> value) {
    logger()->trace("Writing value {} to memory at address {} ", value.to_uint(), address.to_uint());
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
    logger()->trace("execution started");

    while (true) {

#ifdef ENABLE_TESTING
        if (isResetted()) {
            writeReg(SELECT_REG_A, 0);
            writeReg(SELECT_REG_B, 0);
            writeReg(SELECT_REG_C, 0);
            writeReg(SELECT_REG_D, 0);
            writeReg(SELECT_REG_E, 0);
            writeReg(SELECT_REG_H, 0);
            writeReg(SELECT_REG_L, 0);
            doneResetting();
        }
        if (isHalted()) {
            wait();
            continue;
        }
#endif

        logger()->trace("thread triggered @ {}", sc_time_stamp().to_string());

        // fetch & decode & execute
        const uint8_t instruction = readMemAt(pc).to_uint(); // 1 cycle
        const uint8_t opgroup = (instruction >> 6) & 0b00000011;
        const uint8_t opcode = (instruction >> 3) & 0b00000111;
        const uint8_t source = instruction & 0b00000111;
        const uint8_t rp = (instruction >> 4) & 0b00000011;
        const uint8_t rp_opcode = instruction & 0b00001111;


        if(instruction != OP_INST_NOP) {
            logger()->trace("pc [{}] -> {} (group: {}, code: {}, source: {})", 
                pc.to_uint(),
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

            if(source == 0b00000110) {      // MVI ddd,data 
                setRegisterValue(opcode, readMemAt(++pc)); // 1 cycle
                if(source == OP_REG_M) {
                    waitFor(9);
                } else {
                    waitFor(6);
                }
                ++pc;
            } else if(rp_opcode == 0b00000001) { // LXI rp,data
                const sc_dt::sc_uint<8> low = readMemAt(++pc);
                const sc_dt::sc_uint<8> high = readMemAt(++pc);
                switch(rp) {
                    case OP_RP_BC:
                        writeReg(SELECT_REG_B, high);
                        writeReg(SELECT_REG_C, low);
                    break;
                    case OP_RP_DE:
                        writeReg(SELECT_REG_D, high);
                        writeReg(SELECT_REG_E, low);
                    break;
                    case OP_RP_HL:
                        writeReg(SELECT_REG_H, high);
                        writeReg(SELECT_REG_L, low);
                    break;
                    case OP_RP_SP:
                        sp = (high << 8) | low;
                    break;
                }
                ++pc;
            }
            
        break;
    
        case OP_GROUP_MOV:
            if(instruction == OP_INST_HLT) { // HLT
                waitFor(7);
#ifdef ENABLE_TESTING
                /*
                    Once sc_stop() has been called,
                    the simulation enters a "terminated" state, and sc_start() cannot be called
                    again within the same execution context.
                */
                {
                    std::lock_guard guard(mutex);
                    halted = true;
                }
#else
                logger()->info("HLT: Stopping execution...");
                sc_stop();
#endif
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

sc_dt::sc_uint<8> ControlUnit::getRegisterValue(uint8_t regCode) {
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

void ControlUnit::setRegisterValue(uint8_t regCode, sc_dt::sc_uint<8> value) {
    switch (regCode) {
        case OP_REG_A: return writeReg(SELECT_REG_A, value); // 1 cycle
        case OP_REG_B: return writeReg(SELECT_REG_B, value); // 1 cycle
        case OP_REG_C: return writeReg(SELECT_REG_C, value); // 1 cycle
        case OP_REG_D: return writeReg(SELECT_REG_D, value); // 1 cycle
        case OP_REG_E: return writeReg(SELECT_REG_E, value); // 1 cycle
        case OP_REG_H: return writeReg(SELECT_REG_H, value); // 1 cycle
        case OP_REG_L: return writeReg(SELECT_REG_L, value); // 1 cycle
        case OP_REG_M: 
        {
            const sc_dt::sc_uint<8> h = readReg(SELECT_REG_H); // 1 cycle
            const sc_dt::sc_uint<8> l = readReg(SELECT_REG_L); // 1 cycle
            // Get the memory value pointed to by HL
            const sc_dt::sc_uint<16> address = (h << 8) | l;
            // Memory at (HL), read from the bus
            writeMemAt(address, value); // 1 cycle
            break;
        }
        default: break; // Should not happen
    }
}

} // namespace sim

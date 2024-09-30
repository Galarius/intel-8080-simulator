//
//  alu.cpp
//
//  Created by Ilia Shoshin on 28.09.24.
//

#include "alu.hpp"
#include "log.hpp"

using namespace sc_core;
using namespace sc_dt;

namespace {
    auto logger() { return spdlog::get(sim::LogName::alu); }
}

namespace sim {

const uint8_t ALU::OP_ADD = 0b0000;
const uint8_t ALU::OP_ADC = 0b0001;
const uint8_t ALU::OP_SUB = 0b0010;
const uint8_t ALU::OP_SBB = 0b0011;
const uint8_t ALU::OP_ANA = 0b0100;
const uint8_t ALU::OP_XRA = 0b0101;
const uint8_t ALU::OP_ORA = 0b0110;
const uint8_t ALU::OP_CMP = 0b0111;

const uint8_t ALU::FLAG_IDX_ZERO = 0;
const uint8_t ALU::FLAG_IDX_CARRY = 1;
const uint8_t ALU::FLAG_IDX_SIGN = 2;
const uint8_t ALU::FLAG_IDX_PARITY = 3;
const uint8_t ALU::FLAG_IDX_AUX_CARRY = 4;

ALU::ALU(sc_module_name name)
    : sc_module(std::move(name)) {
    SC_METHOD(execute);
    // Ensure the ALU recalculates when any of these change
    sensitive << accumulator << operand << opcode;
}

void ALU::execute() {
    const sc_uint<8> a = accumulator.read();
    const sc_uint<8> b = operand.read();
    const sc_uint<4> op = opcode.read();

    logger()->trace("[->] A={}; arg={}; opcode={}", a.to_int(), b.to_int(), op.to_int());

    // 8-bit accumulator
    sc_dt::sc_uint<8> regACC = 0;
    // 5-bit flag register (Z, C, S, P, AC)
    sc_dt::sc_uint<5> regFR = flags.read();

    switch (op) {
        case ALU::OP_ADD:
            regACC = a + b; 
            regFR[FLAG_IDX_CARRY] = a + b > 255;
            regFR[FLAG_IDX_AUX_CARRY] = ((a & 0x0F) + (b & 0x0F)) > 0x0F;
            regFR[FLAG_IDX_ZERO] = regACC == 0; // Zero flag
            regFR[FLAG_IDX_SIGN] = regACC[7]; // Sign flag (MSB of result)
            regFR[FLAG_IDX_PARITY] = __builtin_parity(regACC); // Calculate parity (even parity)
            break;
        case ALU::OP_ADC:
            regACC = a + b + regFR[FLAG_IDX_CARRY];
            regFR[FLAG_IDX_CARRY] = (a + b + regFR[FLAG_IDX_CARRY]) > 255;
            regFR[FLAG_IDX_ZERO] = regACC == 0;
            regFR[FLAG_IDX_SIGN] = regACC[7];
            regFR[FLAG_IDX_PARITY] = __builtin_parity(regACC);
            break;
        case ALU::OP_SUB:
            regACC = a - b;
            regFR[FLAG_IDX_CARRY] = a < b; // Set Carry flag for subtraction (indicates borrow)
            regFR[FLAG_IDX_AUX_CARRY] = 0;
            regFR[FLAG_IDX_ZERO] = regACC == 0;
            regFR[FLAG_IDX_SIGN] = regACC[7];
            regFR[FLAG_IDX_PARITY] = __builtin_parity(regACC);
            break;
        case ALU::OP_SBB:
            regACC = a - b - regFR[FLAG_IDX_CARRY]; // Subtract arg and Carry
            regFR[FLAG_IDX_CARRY] = a < (b + regFR[FLAG_IDX_CARRY]); // Set Carry flag (borrow)
            regFR[FLAG_IDX_AUX_CARRY] = 0;
            regFR[FLAG_IDX_ZERO] = regACC == 0;
            regFR[FLAG_IDX_SIGN] = regACC[7];
            regFR[FLAG_IDX_PARITY] = __builtin_parity(regACC);
            break;
        case ALU::OP_ANA:
            regACC = a & b;
            regFR[FLAG_IDX_CARRY] = 0;
            regFR[FLAG_IDX_AUX_CARRY] = ((a & 0x0F) & (b & 0x0F)) > 0x0F;
            regFR[FLAG_IDX_ZERO] = regACC == 0;
            regFR[FLAG_IDX_SIGN] = regACC[7];
            regFR[FLAG_IDX_PARITY] = __builtin_parity(regACC);
            break;
        case ALU::OP_XRA:
            regACC = a ^ b;
            regFR[FLAG_IDX_CARRY] = 0;
            regFR[FLAG_IDX_AUX_CARRY] = 0;
            regFR[FLAG_IDX_ZERO] = regACC == 0;
            regFR[FLAG_IDX_SIGN] = regACC[7];
            regFR[FLAG_IDX_PARITY] = __builtin_parity(regACC);
            break;
         case ALU::OP_ORA:
            regACC = a | b;
            regFR[FLAG_IDX_CARRY] = 0;
            regFR[FLAG_IDX_AUX_CARRY] = 0;
            regFR[FLAG_IDX_ZERO] = regACC == 0;
            regFR[FLAG_IDX_SIGN] = regACC[7];
            regFR[FLAG_IDX_PARITY] = __builtin_parity(regACC);
            break;
        case ALU::OP_CMP:
            regACC = a - b; // Internal only
            regFR[FLAG_IDX_CARRY] = a < b;
            regFR[FLAG_IDX_AUX_CARRY] = 0;
            regFR[FLAG_IDX_ZERO] = regACC == 0;
            regFR[FLAG_IDX_SIGN] = regACC[7];
            regFR[FLAG_IDX_PARITY] = __builtin_parity(regACC);
            regACC = 0;
            break;
    }

    logger()->trace("[<-] result={}; flags={}", regACC.to_int(), regFR.to_int());

    result.write(regACC);
    flags.write(regFR);
}

} // namespace sim
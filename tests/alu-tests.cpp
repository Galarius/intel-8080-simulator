//
//  alu-tests.cpp
//
//  Created by Ilia Shoshin on 28.09.24.
//

#include <systemc>
#include <gtest/gtest.h>

#include "modules.hpp"
#include "alu.hpp"

using namespace sc_core;
using namespace sim;

class ALUConsumer final {
    ALU alu{"ALU"};
public:
    // Signal declarations
    sc_core::sc_signal<sc_dt::sc_uint<8>> acumulator;
    sc_core::sc_signal<sc_dt::sc_uint<8>> operand;
    sc_core::sc_signal<sc_dt::sc_uint<4>> opcode;
    sc_core::sc_signal<sc_dt::sc_uint<8>> result;
    sc_core::sc_signal<sc_dt::sc_uint<5>> flags;

    ALUConsumer() {
        // Bind signals to ALU ports
        alu.accumulator(acumulator);
        alu.operand(operand);
        alu.opcode(opcode);
        alu.result(result);
        alu.flags(flags);
    }
};

// We need to create all modules and set all signals before starting any simulations.
static modules::add<ALUConsumer> gALU;

#pragma mark - ADD

TEST(ALUConsumers, AddTest) {
    auto alu = modules::get<ALUConsumer>();
    alu->acumulator.write(10);
    alu->operand.write(15);
    alu->opcode.write(ALU::OP_ADD);
    
    sc_start(1, SC_NS);

    EXPECT_EQ(alu->result.read(), 25);
    EXPECT_EQ(alu->flags.read()[0], 0);
    EXPECT_EQ(alu->flags.read()[1], 0);
}

TEST(ALUConsumers, ZeroResultTest) {
    auto alu = modules::get<ALUConsumer>();
    alu->acumulator.write(0);
    alu->operand.write(0);
    alu->opcode.write(ALU::OP_ADD);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->result.read(), 0);
    EXPECT_EQ(alu->flags.read()[0], 1);
}

TEST(ALUConsumers, OverflowTest) {
    auto alu = modules::get<ALUConsumer>();
    alu->acumulator.write(200);
    alu->operand.write(100);
    alu->opcode.write(ALU::OP_ADD);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->result.read(), 44); // Check result due to overflow (200 + 100 = 300 -> 300 % 256 = 44)
    EXPECT_EQ(alu->flags.read()[1], 1);
}

#pragma mark - ADC

TEST(ALUConsumers, ADCOperationTest) {
    auto alu = modules::get<ALUConsumer>();
    sc_dt::sc_uint<5> flags = 0b00000;
    flags |= (1 << 1); // Set Carry flag

    alu->acumulator.write(100);
    alu->operand.write(100);
    alu->opcode.write(ALU::OP_ADC);
    alu->flags.write(flags);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->result.read(), 201);  // Check result (100 + 100 + 1 = 201)
    EXPECT_EQ(alu->flags.read()[0], 0);
    EXPECT_EQ(alu->flags.read()[1], 0);  // Carry flag should be 0 (no overflow)
}

TEST(ALUConsumers, ADCOverflowTest) {
    auto alu = modules::get<ALUConsumer>();
    sc_dt::sc_uint<5> flags = 0b00000;
    flags |= (1 << 1);

    alu->acumulator.write(250);
    alu->operand.write(10);
    alu->opcode.write(ALU::OP_ADC);
    alu->flags.write(flags);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->result.read(), 5);  // Check result (250 + 10 + 1 = 261, wrap to 5)
    EXPECT_EQ(alu->flags.read()[0], 0); 
    EXPECT_EQ(alu->flags.read()[1], 1); // Carry flag should be 1 (overflow occurred)
}

#pragma mark - SUB

TEST(ALUConsumers, SUBOperationTest) {
    auto alu = modules::get<ALUConsumer>();

    alu->acumulator.write(150);
    alu->operand.write(100);
    alu->opcode.write(ALU::OP_SUB);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->result.read(), 50); // Check result (150 - 100 = 50)
    EXPECT_EQ(alu->flags.read()[0], 0);
    EXPECT_EQ(alu->flags.read()[1], 0); // Carry flag should be 0 (no borrow)
}

TEST(ALUConsumers, SUBBorrowTest) {
    auto alu = modules::get<ALUConsumer>();

    alu->acumulator.write(50);
    alu->operand.write(100);
    alu->opcode.write(ALU::OP_SUB);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->result.read(), 206); // Check result (50 - 100 = -50, wrap to 206 in 8-bit)
    EXPECT_EQ(alu->flags.read()[0], 0); 
    EXPECT_EQ(alu->flags.read()[1], 1);  // Carry flag should be 1 (borrow occurred)
}

#pragma mark - SBB

TEST(ALUConsumers, SBBOperationTest) {
    auto alu = modules::get<ALUConsumer>();
    sc_dt::sc_uint<5> flags = 0b00000;
    flags |= (1 << 1);

    alu->acumulator.write(150);
    alu->operand.write(100);
    alu->flags.write(flags); // Set Carry flag (for the borrow)
    alu->opcode.write(ALU::OP_SBB);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->result.read(), 49); // Check result (150 - 100 - 1 = 49)
    EXPECT_EQ(alu->flags.read()[0], 0);
    EXPECT_EQ(alu->flags.read()[1], 0); // Carry flag should be 0 (no borrow)
}

TEST(ALUConsumers, SBBBorrowTest) {
    auto alu = modules::get<ALUConsumer>();
    sc_dt::sc_uint<5> flags = 0b00000;
    flags |= (1 << 1);

    alu->acumulator.write(50);
    alu->operand.write(100);
    alu->flags.write(flags); // Set Carry flag (for the borrow)
    alu->opcode.write(ALU::OP_SBB);

    sc_start(1, SC_NS);

    // Check result
    EXPECT_EQ(alu->result.read(), 205); // Check result (50 - 100 - 1 = -51, wrap to 205 in 8-bit)
    EXPECT_EQ(alu->flags.read()[0], 0); 
    EXPECT_EQ(alu->flags.read()[1], 1); // Carry flag should be 1 (borrow occurred)
}

#pragma mark - ANA

TEST(ALUConsumers, ANAOperationTest) {
    auto alu = modules::get<ALUConsumer>();

    alu->acumulator.write(0b11001100);
    alu->operand.write(0b10101010);
    alu->opcode.write(ALU::OP_ANA);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->result.read(), 0b10001000); // Check result (0b11001100 AND 0b10101010 = 0b10001000)
    EXPECT_EQ(alu->flags.read()[0], 0);
    EXPECT_EQ(alu->flags.read()[2], 1); // Sign flag should be 1 (result's MSB is 1)
    EXPECT_EQ(alu->flags.read()[3], __builtin_parity(0b10001000));
}

TEST(ALUConsumers, ANAZeroTest) {
    auto alu = modules::get<ALUConsumer>();

    alu->acumulator.write(0b00000000);
    alu->operand.write(0b00000000);
    alu->opcode.write(ALU::OP_ANA);

    sc_start(1, SC_NS);

    // Check result
    EXPECT_EQ(alu->result.read(), 0b00000000); // Check result (0b00000000 AND 0b00000000 = 0b00000000)
    EXPECT_EQ(alu->flags.read()[0], 1); // Zero flag should be 1 (result is zero)
    EXPECT_EQ(alu->flags.read()[2], 0); // Sign flag should be 0 (result's MSB is 0)
    EXPECT_EQ(alu->flags.read()[3], __builtin_parity(0b00000000));
}

#pragma mark - XRA

TEST(ALUConsumers, XRAOperationTest) {
    auto alu = modules::get<ALUConsumer>();

    alu->acumulator.write(0b11001100);
    alu->operand.write(0b10101010);
    alu->opcode.write(ALU::OP_XRA);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->result.read(), 0b01100110);
    EXPECT_EQ(alu->flags.read()[0], 0);
    EXPECT_EQ(alu->flags.read()[2], 0); // Sign flag should be 0 (result's MSB is 0)
    EXPECT_EQ(alu->flags.read()[3], __builtin_parity(0b01100110));
}

TEST(ALUConsumers, XRAZeroTest) {
    auto alu = modules::get<ALUConsumer>();

    alu->acumulator.write(0b11111111);
    alu->operand.write(0b11111111);
    alu->opcode.write(ALU::OP_XRA);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->result.read(), 0b00000000); // Check result (0b11111111 XOR 0b11111111 = 0b00000000)
    EXPECT_EQ(alu->flags.read()[0], 1); // Zero flag should be 1 (result is zero)
    EXPECT_EQ(alu->flags.read()[2], 0); // Sign flag should be 0 (result's MSB is 0)
    EXPECT_EQ(alu->flags.read()[3], __builtin_parity(0b00000000));
}

#pragma mark - ORA

TEST(ALUConsumers, ORAOperationTest) {
    auto alu = modules::get<ALUConsumer>();

    alu->acumulator.write(0b11001100);
    alu->operand.write(0b10101010);
    alu->opcode.write(ALU::OP_ORA);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->result.read(), 0b11101110); // Check result (0b11001100 OR 0b10101010 = 0b11101110)
    EXPECT_EQ(alu->flags.read()[0], 0);
    EXPECT_EQ(alu->flags.read()[2], 1);
    EXPECT_EQ(alu->flags.read()[3], __builtin_parity(0b11101110));
}

TEST(ALUConsumers, ORAZeroTest) {
    auto alu = modules::get<ALUConsumer>();

    alu->acumulator.write(0b00000000);
    alu->operand.write(0b00000000);
    alu->opcode.write(ALU::OP_ORA);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->result.read(), 0b00000000); // Check result (0b00000000 OR 0b00000000 = 0b00000000)
    EXPECT_EQ(alu->flags.read()[0], 1); // Zero flag should be 1 (result is zero)
    EXPECT_EQ(alu->flags.read()[2], 0); // Sign flag should be 0 (result's MSB is 0)
    EXPECT_EQ(alu->flags.read()[3], __builtin_parity(0b00000000));
}

#pragma mark - CMP

TEST(ALUConsumers, CMPOperationTest) {
    auto alu = modules::get<ALUConsumer>();

    alu->acumulator.write(150);
    alu->operand.write(100);
    alu->opcode.write(ALU::OP_CMP);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->result.read(), 0);
    EXPECT_EQ(alu->flags.read()[0], 0);
    EXPECT_EQ(alu->flags.read()[1], 0);
    EXPECT_EQ(alu->flags.read()[2], 0);
}

TEST(ALUConsumers, CMPBorrowTest) {
    auto alu = modules::get<ALUConsumer>();

    alu->acumulator.write(50);
    alu->operand.write(100);
    alu->opcode.write(ALU::OP_CMP);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->result.read(), 0);
    EXPECT_EQ(alu->flags.read()[0], 0);
    EXPECT_EQ(alu->flags.read()[1], 1);
    EXPECT_EQ(alu->flags.read()[2], 1);
}

TEST(ALUConsumers, CMPLessThanTest) {
    auto alu = modules::get<ALUConsumer>();

    alu->acumulator.write(50);
    alu->operand.write(100);
    alu->opcode.write(ALU::OP_CMP);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->result.read(), 0);
    EXPECT_EQ(alu->flags.read()[0], 0); // Zero flag should be 0 (not equal)
    EXPECT_EQ(alu->flags.read()[1], 1); // Carry flag should be 1 (borrow occurred)
    EXPECT_EQ(alu->flags.read()[2], 1); // Sign flag should be 1 (result is negative)
    EXPECT_EQ(alu->flags.read()[3], __builtin_parity(50)); // Parity flag should be based on 50
}
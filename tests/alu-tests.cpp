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

class ALUTest final {
    ALU alu{"ALU"};
public:
    // Signal declarations
    sc_core::sc_signal<sc_dt::sc_uint<8>> sigA;
    sc_core::sc_signal<sc_dt::sc_uint<8>> sigArg;
    sc_core::sc_signal<sc_dt::sc_uint<4>> sigOpCode;
    sc_core::sc_signal<sc_dt::sc_uint<8>> sigResult;
    sc_core::sc_signal<sc_dt::sc_uint<5>> sigFlags;

    ALUTest() {
        // Bind signals to ALU ports
        alu.inA(sigA);
        alu.inArg(sigArg);
        alu.inOpCode(sigOpCode);
        alu.outResult(sigResult);
        alu.outFlags(sigFlags);
    }
};

// We need to create all modules and set all signals before starting any simulations.
static modules::add<ALUTest> gALU;

#pragma mark - ADD

TEST(ALUTests, AddTest) {
    auto alu = modules::get<ALUTest>();
    alu->sigA.write(10);
    alu->sigArg.write(15);
    alu->sigOpCode.write(ALU::OP_ADD);
    
    sc_start(1, SC_NS); // Wait for one time unit

    EXPECT_EQ(alu->sigResult.read(), 25);
    EXPECT_EQ(alu->sigFlags.read()[0], 0);
    EXPECT_EQ(alu->sigFlags.read()[1], 0);
}

TEST(ALUTests, ZeroResultTest) {
    auto alu = modules::get<ALUTest>();
    alu->sigA.write(0);
    alu->sigArg.write(0);
    alu->sigOpCode.write(ALU::OP_ADD);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->sigResult.read(), 0);
    EXPECT_EQ(alu->sigFlags.read()[0], 1);
}

TEST(ALUTests, OverflowTest) {
    auto alu = modules::get<ALUTest>();
    alu->sigA.write(200);
    alu->sigArg.write(100);
    alu->sigOpCode.write(ALU::OP_ADD);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->sigResult.read(), 44); // Check result due to overflow (200 + 100 = 300 -> 300 % 256 = 44)
    EXPECT_EQ(alu->sigFlags.read()[1], 1);
}

#pragma mark - ADC

TEST(ALUTests, ADCOperationTest) {
    auto alu = modules::get<ALUTest>();
    sc_dt::sc_uint<5> flags = 0b00000;
    flags |= (1 << 1); // Set Carry flag

    alu->sigA.write(100);
    alu->sigArg.write(100);
    alu->sigOpCode.write(ALU::OP_ADC);
    alu->sigFlags.write(flags);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->sigResult.read(), 201);  // Check result (100 + 100 + 1 = 201)
    EXPECT_EQ(alu->sigFlags.read()[0], 0);
    EXPECT_EQ(alu->sigFlags.read()[1], 0);  // Carry flag should be 0 (no overflow)
}

TEST(ALUTests, ADCOverflowTest) {
    auto alu = modules::get<ALUTest>();
    sc_dt::sc_uint<5> flags = 0b00000;
    flags |= (1 << 1);

    alu->sigA.write(250);
    alu->sigArg.write(10);
    alu->sigOpCode.write(ALU::OP_ADC);
    alu->sigFlags.write(flags);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->sigResult.read(), 5);  // Check result (250 + 10 + 1 = 261, wrap to 5)
    EXPECT_EQ(alu->sigFlags.read()[0], 0); 
    EXPECT_EQ(alu->sigFlags.read()[1], 1); // Carry flag should be 1 (overflow occurred)
}

#pragma mark - SUB

TEST(ALUTests, SUBOperationTest) {
    auto alu = modules::get<ALUTest>();

    alu->sigA.write(150);
    alu->sigArg.write(100);
    alu->sigOpCode.write(ALU::OP_SUB);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->sigResult.read(), 50); // Check result (150 - 100 = 50)
    EXPECT_EQ(alu->sigFlags.read()[0], 0);
    EXPECT_EQ(alu->sigFlags.read()[1], 0); // Carry flag should be 0 (no borrow)
}

TEST(ALUTests, SUBBorrowTest) {
    auto alu = modules::get<ALUTest>();

    alu->sigA.write(50);
    alu->sigArg.write(100);
    alu->sigOpCode.write(ALU::OP_SUB);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->sigResult.read(), 206); // Check result (50 - 100 = -50, wrap to 206 in 8-bit)
    EXPECT_EQ(alu->sigFlags.read()[0], 0); 
    EXPECT_EQ(alu->sigFlags.read()[1], 1);  // Carry flag should be 1 (borrow occurred)
}

#pragma mark - SBB

TEST(ALUTests, SBBOperationTest) {
    auto alu = modules::get<ALUTest>();
    sc_dt::sc_uint<5> flags = 0b00000;
    flags |= (1 << 1);

    alu->sigA.write(150);
    alu->sigArg.write(100);
    alu->sigFlags.write(flags); // Set Carry flag (for the borrow)
    alu->sigOpCode.write(ALU::OP_SBB);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->sigResult.read(), 49); // Check result (150 - 100 - 1 = 49)
    EXPECT_EQ(alu->sigFlags.read()[0], 0);
    EXPECT_EQ(alu->sigFlags.read()[1], 0); // Carry flag should be 0 (no borrow)
}

TEST(ALUTests, SBBBorrowTest) {
    auto alu = modules::get<ALUTest>();
    sc_dt::sc_uint<5> flags = 0b00000;
    flags |= (1 << 1);

    alu->sigA.write(50);
    alu->sigArg.write(100);
    alu->sigFlags.write(flags); // Set Carry flag (for the borrow)
    alu->sigOpCode.write(ALU::OP_SBB);

    sc_start(1, SC_NS);

    // Check result
    EXPECT_EQ(alu->sigResult.read(), 205); // Check result (50 - 100 - 1 = -51, wrap to 205 in 8-bit)
    EXPECT_EQ(alu->sigFlags.read()[0], 0); 
    EXPECT_EQ(alu->sigFlags.read()[1], 1); // Carry flag should be 1 (borrow occurred)
}

#pragma mark - ANA

TEST(ALUTests, ANAOperationTest) {
    auto alu = modules::get<ALUTest>();

    alu->sigA.write(0b11001100);
    alu->sigArg.write(0b10101010);
    alu->sigOpCode.write(ALU::OP_ANA);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->sigResult.read(), 0b10001000); // Check result (0b11001100 AND 0b10101010 = 0b10001000)
    EXPECT_EQ(alu->sigFlags.read()[0], 0);
    EXPECT_EQ(alu->sigFlags.read()[2], 1); // Sign flag should be 1 (result's MSB is 1)
    EXPECT_EQ(alu->sigFlags.read()[3], __builtin_parity(0b10001000));
}

TEST(ALUTests, ANAZeroTest) {
    auto alu = modules::get<ALUTest>();

    alu->sigA.write(0b00000000);
    alu->sigArg.write(0b00000000);
    alu->sigOpCode.write(ALU::OP_ANA);

    sc_start(1, SC_NS);

    // Check result
    EXPECT_EQ(alu->sigResult.read(), 0b00000000); // Check result (0b00000000 AND 0b00000000 = 0b00000000)
    EXPECT_EQ(alu->sigFlags.read()[0], 1); // Zero flag should be 1 (result is zero)
    EXPECT_EQ(alu->sigFlags.read()[2], 0); // Sign flag should be 0 (result's MSB is 0)
    EXPECT_EQ(alu->sigFlags.read()[3], __builtin_parity(0b00000000));
}

#pragma mark - XRA

TEST(ALUTests, XRAOperationTest) {
    auto alu = modules::get<ALUTest>();

    alu->sigA.write(0b11001100);
    alu->sigArg.write(0b10101010);
    alu->sigOpCode.write(ALU::OP_XRA);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->sigResult.read(), 0b01100110);
    EXPECT_EQ(alu->sigFlags.read()[0], 0);
    EXPECT_EQ(alu->sigFlags.read()[2], 0); // Sign flag should be 0 (result's MSB is 0)
    EXPECT_EQ(alu->sigFlags.read()[3], __builtin_parity(0b01100110));
}

TEST(ALUTests, XRAZeroTest) {
    auto alu = modules::get<ALUTest>();

    alu->sigA.write(0b11111111);
    alu->sigArg.write(0b11111111);
    alu->sigOpCode.write(ALU::OP_XRA);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->sigResult.read(), 0b00000000); // Check result (0b11111111 XOR 0b11111111 = 0b00000000)
    EXPECT_EQ(alu->sigFlags.read()[0], 1); // Zero flag should be 1 (result is zero)
    EXPECT_EQ(alu->sigFlags.read()[2], 0); // Sign flag should be 0 (result's MSB is 0)
    EXPECT_EQ(alu->sigFlags.read()[3], __builtin_parity(0b00000000));
}

#pragma mark - ORA

TEST(ALUTests, ORAOperationTest) {
    auto alu = modules::get<ALUTest>();

    alu->sigA.write(0b11001100);
    alu->sigArg.write(0b10101010);
    alu->sigOpCode.write(ALU::OP_ORA);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->sigResult.read(), 0b11101110); // Check result (0b11001100 OR 0b10101010 = 0b11101110)
    EXPECT_EQ(alu->sigFlags.read()[0], 0);
    EXPECT_EQ(alu->sigFlags.read()[2], 1);
    EXPECT_EQ(alu->sigFlags.read()[3], __builtin_parity(0b11101110));
}

TEST(ALUTests, ORAZeroTest) {
    auto alu = modules::get<ALUTest>();

    alu->sigA.write(0b00000000);
    alu->sigArg.write(0b00000000);
    alu->sigOpCode.write(ALU::OP_ORA);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->sigResult.read(), 0b00000000); // Check result (0b00000000 OR 0b00000000 = 0b00000000)
    EXPECT_EQ(alu->sigFlags.read()[0], 1); // Zero flag should be 1 (result is zero)
    EXPECT_EQ(alu->sigFlags.read()[2], 0); // Sign flag should be 0 (result's MSB is 0)
    EXPECT_EQ(alu->sigFlags.read()[3], __builtin_parity(0b00000000));
}

#pragma mark - CMP

TEST(ALUTests, CMPOperationTest) {
    auto alu = modules::get<ALUTest>();

    alu->sigA.write(150);
    alu->sigArg.write(100);
    alu->sigOpCode.write(ALU::OP_CMP);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->sigResult.read(), 0);
    EXPECT_EQ(alu->sigFlags.read()[0], 0);
    EXPECT_EQ(alu->sigFlags.read()[1], 0);
    EXPECT_EQ(alu->sigFlags.read()[2], 0);
}

TEST(ALUTests, CMPBorrowTest) {
    auto alu = modules::get<ALUTest>();

    alu->sigA.write(50);
    alu->sigArg.write(100);
    alu->sigOpCode.write(ALU::OP_CMP);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->sigResult.read(), 0);
    EXPECT_EQ(alu->sigFlags.read()[0], 0);
    EXPECT_EQ(alu->sigFlags.read()[1], 1);
    EXPECT_EQ(alu->sigFlags.read()[2], 1);
}

TEST(ALUTests, CMPLessThanTest) {
    auto alu = modules::get<ALUTest>();

    alu->sigA.write(50);
    alu->sigArg.write(100);
    alu->sigOpCode.write(ALU::OP_CMP);

    sc_start(1, SC_NS);

    EXPECT_EQ(alu->sigResult.read(), 0);
    EXPECT_EQ(alu->sigFlags.read()[0], 0); // Zero flag should be 0 (not equal)
    EXPECT_EQ(alu->sigFlags.read()[1], 1); // Carry flag should be 1 (borrow occurred)
    EXPECT_EQ(alu->sigFlags.read()[2], 1); // Sign flag should be 1 (result is negative)
    EXPECT_EQ(alu->sigFlags.read()[3], __builtin_parity(50)); // Parity flag should be based on 50
}
//
//  processor-tests.cpp
//
//  Created by Ilia Shoshin on 28.09.24.
//

#include <systemc>
#include <gtest/gtest.h>

#include "modules.hpp"
#include "processor.hpp"

using namespace sc_core;
using namespace sim;

class Intel8080Consumer final {
public:
    Intel8080 processor {"Intel8080"};

    void loadMemory(const std::array<uint8_t, DEFAULT_MEMORY_SIZE>& data) {
        processor.loadMemory(data);
    }
};

// We need to create all modules and set all signals before starting any simulations.
static modules::add<Intel8080Consumer> gProcessor;

#pragma mark - Control Unit Tests

TEST(ProcessorTests, ADIInstructionTest) {
    auto consumer = modules::get<Intel8080Consumer>();
    auto& processor = consumer->processor;
    
    std::array<uint8_t, DEFAULT_MEMORY_SIZE> program = {
        0b11000110, 0b00000101, // ADI 5
        0b01110110,             // HLT
    };
    processor.loadMemory(program);

    sc_start();

    auto expectedValue = processor.registerA.dataOut.read();
    EXPECT_EQ(expectedValue.to_uint(), 5);
}
//
//  processor-tests.cpp
//
//  Created by Ilia Shoshin on 28.09.24.
//

#include <systemc>
#include <gtest/gtest.h>
#include <thread>
#include <chrono>  // For sleep

#include "log.hpp"
#include "modules.hpp"
#include "processor.hpp"

using namespace sc_core;
using namespace sc_dt;
using namespace sim;

// We need to create all modules and set all signals before starting any simulations.
static modules::add<Intel8080, sc_module_name> gProcessor ("Intel8080TestBench", "Intel8080");

#pragma mark - Processor Tests

namespace  {

constexpr int simulationTime = 3; // sec
constexpr int waitTimeout = 10; // sec

class ProcessorTests : public ::testing::Test {

    static std::shared_ptr<std::thread> simulationThread;

protected:

    static void SetUpTestSuite() {
        // Run sc_start in a separate thread
        ProcessorTests::simulationThread = std::make_shared<std::thread>([]() {
            sc_start(simulationTime, SC_SEC);
        });
    }

    static void TearDownTestSuite() {
        // Ensure the simulation thread finishes
        if (ProcessorTests::simulationThread->joinable()) {
            ProcessorTests::simulationThread->join();
        }
    }
};

std::shared_ptr<std::thread> ProcessorTests::simulationThread = nullptr;

}

namespace {
    // Unit is SC_SEC
    bool WaitForHalt(int timeout, std::shared_ptr<Intel8080> processor) {
        static const int sleepInterval = 0.1;  // Check every 100 ms

        // Wait for the processor to halt (polling isHalted)
        int waited = 0;
        while (!processor->cu.isHalted() && waited < timeout) {
            std::this_thread::sleep_for(std::chrono::seconds(sleepInterval));
            waited += sleepInterval;
        }

        return processor->cu.isHalted();
    }
}

TEST_F(ProcessorTests, MVIInstructionTest) {
    spdlog::get(sim::LogName::main)->info("ProcessorTests.MVIInstructionTest\n");

    auto processor = modules::get<Intel8080>("Intel8080TestBench");

    std::array<uint8_t, DEFAULT_MEMORY_SIZE> program = {
        0b00000000,      // NOP
        0b00000110, 18,  // MVI B, 18
        0b00001110, 19,  // MVI C, 19
        0b00010110, 20,  // MVI D, 20
        0b00011110, 21,  // MVI E, 21
        0b00100110, 22,  // MVI H, 22
        0b00101110, 23,  // MVI L, 23
        0b00111110, 24,  // MVI A, 24
        0b01110110       // HLT
    };

    processor->loadMemory(program);  // Load the program into memory

    EXPECT_TRUE(WaitForHalt(waitTimeout, processor));

    EXPECT_EQ(processor->registerB.getValue(), 18);
    EXPECT_EQ(processor->registerC.getValue(), 19);
    EXPECT_EQ(processor->registerD.getValue(), 20);
    EXPECT_EQ(processor->registerE.getValue(), 21);
    EXPECT_EQ(processor->registerH.getValue(), 22);
    EXPECT_EQ(processor->registerL.getValue(), 23);
    EXPECT_EQ(processor->registerA.getValue(), 24);

    EXPECT_EQ(processor->cu.getPC(), 15);
}

TEST_F(ProcessorTests, MVI_M_InstructionTest) {
    spdlog::get(sim::LogName::main)->info("ProcessorTests.MVI_M_InstructionTest\n");

    auto processor = modules::get<Intel8080>("Intel8080TestBench");
    std::array<uint8_t, DEFAULT_MEMORY_SIZE> program = {
        0b00000000,       // NOP
        0b00100110, 0b00000001,  // MVI H, 0b00000001 (H = 0x01)
        0b00101110, 0b00001000,  // MVI L, 0b00001000 (L = 0x08)
        0b00110110, 117,         // MVI M, 117
        0b01110110               // HLT
    };

    processor->loadMemory(program);

    EXPECT_TRUE(WaitForHalt(waitTimeout, processor));

    const auto h = processor->registerH.getValue();
    const auto l = processor->registerL.getValue();
    // Memory address formed by HL (H = 0x01, L = 0x08 -> HL = 0x0108)
    const uint16_t memoryAddress = (h.to_uint() << 8) | l.to_uint();
    
    EXPECT_EQ(h, 0b00000001);
    EXPECT_EQ(l, 0b00001000);
    EXPECT_EQ(memoryAddress, 0x0108);
    EXPECT_EQ(processor->memory.getValueAt(memoryAddress), 117);

}

 TEST_F(ProcessorTests, ADIInstructionTest) {
     auto processor = modules::get<Intel8080>("Intel8080TestBench");
    
     std::array<uint8_t, DEFAULT_MEMORY_SIZE> program = {
         0b00000000,       // NOP
         0b11000110, 0b00000101, // ADI 5
         0b01110110,             // HLT
     };
     processor->loadMemory(program);

     EXPECT_TRUE(WaitForHalt(waitTimeout, processor));

     EXPECT_EQ(processor->registerA.getValue(), 5);
 }

 TEST_F(ProcessorTests, LXIInstructionTest) {
     auto processor = modules::get<Intel8080>("Intel8080TestBench");
    
     std::array<uint8_t, DEFAULT_MEMORY_SIZE> program = {
         0b00000000,       // NOP
         0b00000001, 5, 7, // LXI 5 7 (B <- 7, C <- 5)
         0b00010001, 3, 9, // LXI 3 9 (D <- 9, E <- 3)
         0b00100001, 6, 2, // LXI 6 2 (H <- 2, L <- 6)
         0b00110001, 0x34, 0x12, // LXI 0x34 0x12 (SP <- 0x1234)
         0b01110110,       // HLT
     };
     processor->loadMemory(program);

     EXPECT_TRUE(WaitForHalt(waitTimeout, processor));

     EXPECT_EQ(processor->registerB.getValue(), 7);
     EXPECT_EQ(processor->registerC.getValue(), 5);
     EXPECT_EQ(processor->registerD.getValue(), 9);
     EXPECT_EQ(processor->registerE.getValue(), 3);
     EXPECT_EQ(processor->registerH.getValue(), 2);
     EXPECT_EQ(processor->registerL.getValue(), 6);
     EXPECT_EQ(processor->cu.getSP(), 0x1234);
 }

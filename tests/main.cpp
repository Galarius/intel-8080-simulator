//
//  main.cpp
//
//  Created by Ilia Shoshin on 29/09/24.
//

#include "log.hpp"
#include "modules.hpp"
#include <systemc>
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
    sim::ConfigureNullLogging();
    ::testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();
    sc_core::sc_stop();
    modules::shutdown();
    spdlog::shutdown();
    return result;
}
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
    //sim::ConfigureFileLogging("simulator-tests.log", spdlog::level::trace);
    sim::ConfigureNullLogging();

    ::testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();

    if(sc_core::sc_is_running()) {
        sc_core::sc_stop();
    }
    modules::shutdown();
    
    spdlog::get(sim::LogName::main)->info("Shutting down...\n\n");
    spdlog::shutdown();

    return result;
}

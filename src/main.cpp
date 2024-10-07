#include "processor.hpp"
#include "log.hpp"

#include <systemc>

using namespace sim;

namespace {
    auto logger() { return spdlog::get(LogName::main); }
}

int sc_main(__unused int argc, __unused char* argv[]) {
    ConfigureFileLogging("simulator.log", spdlog::level::trace);

    Intel8080 processor("Intel8080");
    sc_core::sc_start();

    logger()->info("Shutting down...\n\n");
    spdlog::shutdown();

    return 0;
}

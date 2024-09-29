//
//  logs.cpp
//
//  Created by Ilia Shoshin on 28/09/24.
//

#include "log.hpp"
#include "utils.hpp"

#if defined(__APPLE__)
#include "oslogger.hpp"
#endif

#include <iostream>

#include "spdlog/pattern_formatter.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/null_sink.h>

namespace sim {

std::string LogName::main = "sim";
std::string LogName::alu = "alu";
std::string LogName::memory = "memory";
std::string LogName::cu = "cu";

const int flushIntervalSec = 5;

void ConfigureLogging(bool fileLogging, const std::string& filename, spdlog::level::level_enum level)
{
    try
    {
        std::vector<spdlog::sink_ptr> sinks;
        if (fileLogging)
        {
            sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename));
        }
        else
        {
            sinks.push_back(std::make_shared<spdlog::sinks::null_sink_mt>());
        }
#if defined(__APPLE__)
        sinks.push_back(std::make_shared<sim::oslogger_sink_mt>());
#endif

        spdlog::set_default_logger(std::make_shared<spdlog::logger>(LogName::main, sinks.begin(), sinks.end()));
        spdlog::set_level(level);
        std::vector<std::shared_ptr<spdlog::logger>> subLoggers = {
            std::make_shared<spdlog::logger>(LogName::alu, sinks.begin(), sinks.end()),
            std::make_shared<spdlog::logger>(LogName::memory, sinks.begin(), sinks.end()),
            std::make_shared<spdlog::logger>(LogName::cu, sinks.begin(), sinks.end()),
        };

        for (const auto& logger : subLoggers)
        {
            logger->set_level(level);
            spdlog::register_logger(logger);
        }

        if (fileLogging)
        {
            spdlog::flush_on(level);
            spdlog::flush_every(std::chrono::seconds(flushIntervalSec));
        }

        spdlog::get(LogName::main)->info("Starting new session at {}...\n", utils::GetCurrentDateTime());
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cerr << "Log init failed: " << ex.what() << std::endl;
    }
}

void ConfigureFileLogging(const std::string& filename, spdlog::level::level_enum level)
{
    ConfigureLogging(true, filename, level);
}

void ConfigureNullLogging()
{
    ConfigureLogging(false, "", spdlog::level::trace);
}

} // namespace sim

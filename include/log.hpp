//
//  logs.hpp
//
//  Created by Ilia Shoshin on 28/09/24.
//
#pragma once

#include <string>
#include <spdlog/spdlog.h>

namespace sim {

struct LogName
{
    static std::string main;
    static std::string alu;
    static std::string memory;
};

extern void ConfigureFileLogging(const std::string& filename, spdlog::level::level_enum level);
extern void ConfigureNullLogging();

} // namespace sim

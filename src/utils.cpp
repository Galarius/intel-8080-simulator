//
//  utils.cpp
//
//  Created by Ilia Shoshin on 28/09/24.
//

#include "utils.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>

namespace sim::utils {

std::string GetCurrentDateTime() 
{
    auto now = std::chrono::system_clock::now();
    auto itt = std::chrono::system_clock::to_time_t(now);
    std::tm tm = {};

#if defined(WIN32)
    gmtime_s(&tm, &itt);
#else
    tm = *gmtime(&itt);
#endif

    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

} // namespace sim::utils

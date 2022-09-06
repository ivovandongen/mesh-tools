#pragma once

#include <spdlog/spdlog.h>

namespace meshtools::logging {

enum class Level { DEBUG, WARN, INFO, ERROR };

void setLevel(const Level& level);

template<typename... Args>
inline void debug(const char* fmt, const Args&... args) {
    spdlog::debug(fmt, args...);
}

template<typename... Args>
inline void info(const char* fmt, const Args&... args) {
    spdlog::info(fmt, args...);
}

template<typename... Args>
inline void warn(const char* fmt, const Args&... args) {
    spdlog::warn(fmt, args...);
}

template<typename... Args>
inline void error(const char* fmt, const Args&... args) {
    spdlog::error(fmt, args...);
}

} // namespace meshtools::logging
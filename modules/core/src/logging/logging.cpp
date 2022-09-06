#include <meshtools/logging.hpp>

namespace meshtools::logging {

void setLevel(const Level& level) {
    spdlog::set_level([&level]() {
        switch (level) {
            case Level::DEBUG:
                return spdlog::level::debug;
            case Level::WARN:
                return spdlog::level::warn;
            case Level::ERROR:
                return spdlog::level::err;
            case Level::INFO:
                return spdlog::level::info;
            default:
                return spdlog::level::info;
        }
    }());
}

} // namespace meshtools::logging
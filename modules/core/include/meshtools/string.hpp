#pragma once

#include <locale>
#include <string>

namespace meshtools::string {

inline bool endsWith(const std::string& input, const std::string& suffix) {
    if (input.length() >= suffix.length()) {
        return (0 == input.compare(input.length() - suffix.length(), suffix.length(), suffix));
    } else {
        return false;
    }
}

inline size_t search(const std::string& input, const std::string& phrase, const std::locale& locale = std::locale()) {
    auto it = std::search(input.begin(), input.end(), phrase.begin(), phrase.end(), [&](const auto& char1, const auto& char2) {
        return std::toupper(char1, locale) == std::toupper(char2, locale);
    });
    if (it != input.end()) {
        return it - input.begin();
    } else {
        return std::string::npos;
    }
}

} // namespace meshtools::string

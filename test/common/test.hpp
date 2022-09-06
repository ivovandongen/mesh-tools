#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>

static std::filesystem::path getFixturesPath(const std::filesystem::path& relative) {
    return FIXTURES_DIR / relative;
}

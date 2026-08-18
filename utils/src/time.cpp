// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 prunus-avium

#include <utils.hpp>

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>

#include <fmt/base.h>
#include <fmt/format.h>

namespace cerasum::utils::time {
std::string formatTime(std::string_view format, std::chrono::milliseconds milliseconds) {
    uint64_t hours = std::chrono::duration_cast<std::chrono::hours>(milliseconds).count();
    uint64_t minutes = std::chrono::duration_cast<std::chrono::minutes>(milliseconds).count() % 60;
    uint64_t seconds = std::chrono::duration_cast<std::chrono::seconds>(milliseconds).count() % 60;

    return fmt::vformat(format, fmt::make_format_args(hours, minutes, seconds));
}
} // namespace cerasum::utils::time
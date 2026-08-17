// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2026 prunus-avium

/**
 * @file utils.cpp
 * @brief Implementation of utility functions for Cerasum audio player
 *
 * This source file implements the utility functions defined in utils.hpp.
 * It provides functionality for retrieving setting values and managing
 * various utility operations in the Cerasum audio player application.
 *
 * @author prunus-avium
 */

#include <utils.hpp>

#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

namespace cerasum::utils {
nlohmann::json getSetting() {
    nlohmann::json setting;
    std::filesystem::path settingPath = std::filesystem::current_path() / "setting.json";
    if (!std::filesystem::exists(settingPath)) {
        setting["lang"] = "en_US";

        std::ofstream(settingPath) << setting.dump(4);
        return setting;
    }
    std::ifstream(settingPath) >> setting;
    return setting;
}
} // namespace cerasum::utils
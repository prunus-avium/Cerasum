// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2026 prunus-avium

/**
 * @file lang.cpp
 * @brief 实用工具模块语言功能实现
 *
 * @author prunus-avium
 */

#include <utils.hpp>

#include <filesystem>
#include <fstream>
#include <string>

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

namespace cerasum::utils::lang {
nlohmann::json getLang() {
    std::filesystem::path defaultLangPath = std::filesystem::current_path() / "lang" / "en_US.json";

    nlohmann::json langJson;

    nlohmann::json setting = getSetting();
    std::string lang = setting["lang"];
    lang += ".json";

    std::filesystem::path langPath = std::filesystem::current_path() / "lang" / lang;
    if (!std::filesystem::exists(langPath)) {
        if (langPath == defaultLangPath) {
            langJson["error"]["unknown"] = "Unknown error code";
            return langJson;
        }
        langPath = defaultLangPath;
    }

    std::ifstream(langPath) >> langJson;
    return langJson;
}
} // namespace cerasum::utils::lang
// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2026 prunus-avium

/**
 * @file error.cpp
 * @brief 实用工具模块错误处理功能实现
 *
 * @author prunus-avium
 */

#include <utils.hpp>

#include <string>

namespace cerasum::utils::error {
std::string getError(ErrorCode code) {
    auto langJson = cerasum::utils::lang::getLang();

    switch (code) {
    default:
        return langJson["error"]["unknown"].get<std::string>();
    }
}
} // namespace cerasum::utils::error
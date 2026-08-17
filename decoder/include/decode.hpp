// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2026 prunus-avium

#pragma once

#include <any>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <memory>

#include <utils.hpp>

namespace cerasum::decoder {
struct Stream {
    std::unique_ptr<uint8_t[]> data;    //NOLINT
    int sampleRate;
    int channels;
};

std::expected<Stream, utils::error::ErrorCode> decode(const std::filesystem::path &file);

std::expected<std::any, utils::error::ErrorCode> encode(const Stream &pcm);
}
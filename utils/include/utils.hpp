// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 prunus-avium

#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

namespace cerasum::utils {
nlohmann::json getSetting();
} // namespace cerasum::utils

namespace cerasum::utils::error {
enum class ErrorCode : uint8_t {
    Unknown,
    FatalUnknown,
    FileNotFound,
    PermissionDenied,
    IOError,
    DiskFull,
    OutOfMemory,
    InvalidArgument,
    CorruptedData,
    NetworkError,
    CodecNotFound,
    FormatNotSupported,
    EncoderNotFound,
    MuxerNotFound,
    FilterNotFound,
    ProtocolNotSupported,
    StreamNotFound,
    BitstreamFilterNotFound,
    OptionNotFound,
    EndOfFile,
    Aborted,
    ExternalLibraryError,
    InternalBug,
    NotImplemented,
    ExperimentalNotEnabled,
    BufferTooSmall,
    HttpBadRequest,
    HttpUnauthorized,
    HttpForbidden,
    HttpNotFound,
    HttpClientError,
    HttpServerError,
    InputFormatChanged,
    OutputFormatChanged,
    AllocateContextFault,
    AllocateBufferFault,
    NoPcmData,
    Succuss
};
std::string getError(ErrorCode code);
} // namespace cerasum::utils::error

namespace cerasum::utils::lang {
nlohmann::json getLang();
} // namespace cerasum::utils::lang

namespace cerasum::utils::log {
enum class LogLevel : uint8_t {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class Log {
private:
    class Impl;
    std::shared_ptr<Impl> _impl;

public:
    Log(const std::filesystem::path &file, std::string_view tag);
    ~Log() = default;

    void log(LogLevel level, std::string_view message);
    template <typename... Args> void log(LogLevel level, std::string_view format, Args... args) {
        std::string message = fmt::vformat(format, fmt::make_format_args(args...));
        log(level, message);
    }

    static void logToConsole(LogLevel level, std::string_view message);
    template <typename... Args> static void logToConsole(LogLevel level, std::string_view format, Args... args) {
        std::string message = fmt::vformat(format, fmt::make_format_args(args...));
        logToConsole(level, message);
    }

    static void logToFile(LogLevel level, const std::filesystem::path &file, std::string_view message);
    template <typename... Args>
    static void logToFile(LogLevel level, const std::filesystem::path &file, std::string_view format, Args... args) {
        std::string message = fmt::vformat(format, fmt::make_format_args(args...));
        logToFile(level, file, message);
    }
};
} // namespace cerasum::utils::log

namespace cerasum::utils::time {
std::string formatTime(std::string_view format, std::chrono::milliseconds milliseconds);
} // namespace cerasum::utils::time
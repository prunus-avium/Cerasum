// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2026 prunus-avium

/**
 * @file utils.h
 * @brief 实用工具模块
 *
 * 此模块包含utils命名空间
 * - utils: 其他功能命名空间以及获取设置
 *
 * 此命名空间包含以下命名空间
 * - error: 错误码及其处理
 * - lang: 获取语言
 * - log: 日志输出
 * - time: 时间格式化
 *
 * @author prunus-avium
 */

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

/**
 * @brief 实用工具命名空间
 *
 * 此命名空间包含许多使用函数和类
 * 提供错误、语言、日志及时间等方面的支持
 */
namespace cerasum::utils {
/**
 * @brief 获取当前设置
 *
 * @return 一个包含当前设置的JSON对象
 */
nlohmann::json getSetting();
} // namespace cerasum::utils

/**
 * @brief 错误处理功能
 */
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
/**
 * @brief 将错误码转换为可读文本
 *
 * @param code 错误码
 * @return 描述错误的可读字符串
 */
std::string getError(ErrorCode code);
} // namespace cerasum::utils::error

/**
 * @brief 语言设置相关功能
 */
namespace cerasum::utils::lang {
/**
 * @brief 获取当前语言配置
 *
 * @return 一个包含当前语言配置的JSON对象
 */
nlohmann::json getLang();
} // namespace cerasum::utils::lang

/**
 * @brief 日志相关功能
 */
namespace cerasum::utils::log {
/**
 * @brief 日志级别枚举
 *
 * 此枚举定义了可使用的各种日志级别，
 * 从轻到重进行排序
 *
 * - DEBUG: 调试级别，应当在开发阶段使用
 * - INFO: 信息级别，用于报告程序运行状态
 * - WARNING: 警告级别，用于报告不会引起程序发生错误的潜在威胁
 * - ERROR: 错误级别，用于报告程序错误
 * - FATAL: 致命错误级别，用于报告程序崩溃
 *
 */
enum class LogLevel : uint8_t {
    DEBUG,   ///< 调试级别
    INFO,    ///< 信息级别
    WARNING, ///< 警告级别
    ERROR,   ///< 错误级别
    FATAL    ///< 致命错误级别
};

/**
 * @brief 一个用于控制台和文件日志记录的功能类，使用fmt风格
 */
class Log {
private:
    class Impl;
    std::shared_ptr<Impl> _impl;

public:
    Log(const std::filesystem::path &file, std::string_view tag);
    ~Log() = default;

    /**
     * @brief 以指定的日志级别输出一段消息
     *
     * @param level 日志级别
     * @param message 日志消息
     */
    void log(LogLevel level, std::string_view message);
    /**
     * @brief 以指定的日志级别输出一段格式化消息
     *
     * @param level 日志级别
     * @param format 格式字符串
     * @param args 将被格式化到消息中的参数
     */
    template <typename... Args> void log(LogLevel level, std::string_view format, Args... args) {
        std::string message = fmt::vformat(format, fmt::make_format_args(args...));
        log(level, message);
    }

    /**
     * @brief 以指定的日志级别将消息输出至控制台
     *
     * @param level 日志级别
     * @param message 日志消息
     */
    static void logToConsole(LogLevel level, std::string_view message);
    /**
     * @brief 以指定的日志级别将格式化消息输出至控制台
     *
     * @param level 日志级别
     * @param format 格式字符串
     * @param args 将被格式化到消息中的参数
     */
    template <typename... Args> static void logToConsole(LogLevel level, std::string_view format, Args... args) {
        std::string message = fmt::vformat(format, fmt::make_format_args(args...));
        logToConsole(level, message);
    }

    /**
     * @brief 以指定的日志级别将消息写入指定文件中
     *
     * @param level 日志级别
     * @param path 日志文件路径
     * @param message 日志消息
     */
    static void logToFile(LogLevel level, const std::filesystem::path &file, std::string_view message);
    /**
     * @brief 以指定的日志级别将格式化消息写入指定文件中
     *
     * @param level 日志级别
     * @param path 日志文件路径
     * @param format 格式字符串
     * @param args 将被格式化到消息中的参数
     */
    template <typename... Args>
    static void logToFile(LogLevel level, const std::filesystem::path &file, std::string_view format, Args... args) {
        std::string message = fmt::vformat(format, fmt::make_format_args(args...));
        logToFile(level, file, message);
    }
};
} // namespace cerasum::utils::log

/**
 * @brief 时间相关功能
 */
namespace cerasum::utils::time {
/**
 * @brief 以提供的格式，将以毫秒为单位的时间段格式化
 *
 * @param format 格式字符串
 * @param milliseconds 将被格式化的时间段，以毫秒为单位
 * @return 格式化字符串
 */
std::string formatTime(std::string_view format, std::chrono::milliseconds milliseconds);
} // namespace cerasum::utils::time
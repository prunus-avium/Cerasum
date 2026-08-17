// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2026 prunus-avium

/**
 * @file log.cpp
 * @brief 实用工具模块日志功能实现
 *
 * @author prunus-avium
 */

#include <utils.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <spdlog/common.h>
#include <spdlog/details/thread_pool.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

static auto thread_pool = std::make_shared<spdlog::details::thread_pool>(8192, 4);

namespace cerasum::utils::log {
static spdlog::level::level_enum toSpdLogLevel(LogLevel level) {
    switch (level) {
    case LogLevel::DEBUG:
        return spdlog::level::debug;
    case LogLevel::INFO:
        return spdlog::level::info;
    case LogLevel::WARNING:
        return spdlog::level::warn;
    case LogLevel::ERROR:
        return spdlog::level::err;
    case LogLevel::FATAL:
        return spdlog::level::critical;
    default:
        return spdlog::level::info;
    }
}

class Log::Impl {
public:
    std::shared_ptr<spdlog::logger> logger;
};

Log::Log(const std::filesystem::path &file, std::string_view tag)
    : _impl(std::make_shared<Impl>()) {
    auto logTag = std::string(tag);
    auto existing = spdlog::get(logTag);
    if (existing) {
        _impl->logger = existing;
        return;
    }

    std::vector<spdlog::sink_ptr> sinks;
    sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(file.string(), true));
    sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

    _impl->logger = std::make_shared<spdlog::async_logger>(logTag, sinks.begin(), sinks.end(), thread_pool,
                                                           spdlog::async_overflow_policy::block);

    _impl->logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v");
    spdlog::register_logger(_impl->logger);
}

void Log::log(LogLevel level, std::string_view message) { _impl->logger->log(toSpdLogLevel(level), message); }

void Log::logToConsole(LogLevel level, std::string_view message) { spdlog::log(toSpdLogLevel(level), message); }

void Log::logToFile(LogLevel level, const std::filesystem::path &file, std::string_view message) {
    auto logger = spdlog::get(file.string());
    if (!logger) {
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(file.string(), true);
        logger = std::make_shared<spdlog::async_logger>(file.string(), sink, thread_pool,
                                                        spdlog::async_overflow_policy::block);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v");
        spdlog::register_logger(logger);
    }
    logger->log(toSpdLogLevel(level), message);
}
} // namespace cerasum::utils::log
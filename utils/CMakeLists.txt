# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 prunus-avium

add_library(utils STATIC)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lang.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/time.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/error.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cpp
)

target_sources(utils PRIVATE ${SOURCES})

target_include_directories(utils PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(utils PUBLIC
    fmt::fmt
    spdlog::spdlog
    nlohmann_json::nlohmann_json
)

if (MSVC)
    target_compile_options(utils PRIVATE
        /W4
        /permissive-
        /WX
        /utf-8
        $<$<CONFIG:Debug>:/Zi /Od>
        $<$<CONFIG:Release>:/O2 /DNDEBUG>
    )
else()
    target_compile_options(utils PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O2 -DNDEBUG>
    )
endif()
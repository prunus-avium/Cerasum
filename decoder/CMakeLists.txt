add_library(decoder STATIC)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/decode.cpp
)

target_sources(decoder PRIVATE ${SOURCES})

target_include_directories(decoder PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(decoder PUBLIC
    ffmpeg_interface
    utils
)

if (MSVC)
    target_compile_options(decoder PRIVATE
        /W4
        /permissive-
        /WX
        /utf-8
        $<$<CONFIG:Debug>:/Zi /Od>
        $<$<CONFIG:Release>:/O2 /DNDEBUG>
    )
else()
    target_compile_options(decoder PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O2 -DNDEBUG>
    )
endif()
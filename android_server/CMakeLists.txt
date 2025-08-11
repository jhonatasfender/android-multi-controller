cmake_minimum_required(VERSION 3.22)

project(android_server 
    VERSION 1.0.0
    DESCRIPTION "Android Native Streaming Server"
    LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(ANDROID)
    set(CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION clang)
    set(CMAKE_ANDROID_STL_TYPE c++_shared)
    
    find_library(log-lib log)
    find_library(android-lib android)
    find_library(mediandk-lib mediandk)
    find_library(EGL-lib EGL)
    find_library(GLESv2-lib GLESv2)
    
    set(ANDROID_LIBRARIES
        ${log-lib}
        ${android-lib}
        ${mediandk-lib}
        ${EGL-lib}
        ${GLESv2-lib}
    )
else()
    message(WARNING "This project is designed for Android NDK. Use Android toolchain.")
endif()

include_directories(src)

set(COMMON_SOURCES
    src/common/types.h
    src/common/logger.h
)

set(PROTOCOL_SOURCES
    src/protocol/packet_types.h
    src/protocol/protocol_handler.h
)

set(VIDEO_ENCODER_SOURCES
    src/video_encoder/encoder_interface.h
    src/video_encoder/encoder_config.h
    src/video_encoder/mediacodec_h264_encoder.h
    src/video_encoder/mediacodec_h264_encoder.cpp
)

set(SCREEN_CAPTURE_SOURCES
    src/screen_capture/capture_interface.h
    src/screen_capture/media_projection_capture.h
    src/screen_capture/media_projection_capture.cpp
)

set(NETWORK_SOURCES
    src/network/tcp_server.h
    src/network/tcp_server.cpp
)

set(AUDIO_CAPTURE_SOURCES
    src/audio_capture/audio_capture_interface.h
)

set(SERVER_SOURCES
    src/server.h
    src/server.cpp
)

add_executable(android_server
    src/main.cpp
    ${COMMON_SOURCES}
    ${PROTOCOL_SOURCES}
    ${VIDEO_ENCODER_SOURCES}
    ${SCREEN_CAPTURE_SOURCES}
    ${NETWORK_SOURCES}
    ${AUDIO_CAPTURE_SOURCES}
    ${SERVER_SOURCES}
)

target_compile_definitions(android_server PRIVATE
    ANDROID
    __ANDROID__
    ANDROID_STL=c++_shared
)

target_compile_options(android_server PRIVATE
    -Wall
    -Wextra
    -fPIC
    -ffunction-sections
    -fdata-sections
    -funwind-tables
    -fstack-protector-strong
    -no-canonical-prefixes
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(android_server PRIVATE -g -O0)
    target_compile_definitions(android_server PRIVATE DEBUG)
else()
    target_compile_options(android_server PRIVATE -O2 -DNDEBUG)
    target_link_options(android_server PRIVATE -Wl,--gc-sections)
endif()

if(ANDROID)
    target_link_libraries(android_server ${ANDROID_LIBRARIES})
endif()

target_include_directories(android_server PRIVATE
    src
    src/common
    src/protocol
    src/video_encoder
    src/screen_capture
    src/network
    src/audio_capture
) 
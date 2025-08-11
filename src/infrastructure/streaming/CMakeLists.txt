set(STREAMING_SOURCES
    tcp_client.cpp
    h264_decoder.cpp
    streaming_widget.cpp
    native_streaming_service.cpp
)

set(STREAMING_HEADERS
    tcp_client.h
    h264_decoder.h
    streaming_widget.h
    native_streaming_service.h
)

add_library(streaming STATIC
    ${STREAMING_SOURCES}
    ${STREAMING_HEADERS}
)

target_include_directories(streaming PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswscale)

target_link_libraries(streaming
    core
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    ${FFMPEG_LIBRARIES}
)

target_include_directories(streaming PRIVATE
    ${FFMPEG_INCLUDE_DIRS}
)

target_compile_definitions(streaming PRIVATE
    ${FFMPEG_CFLAGS_OTHER}
) 
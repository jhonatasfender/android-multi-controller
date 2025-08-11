cmake_minimum_required(VERSION 3.16)
project(MultiDeviceAndroidController VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to build Android server
option(BUILD_ANDROID_SERVER "Build Android native server" OFF)

set(CMAKE_PREFIX_PATH "~/Qt/6.6.0/gcc_64")

find_package(Qt6 REQUIRED COMPONENTS Core Quick Widgets Network QuickControls2)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include_directories(${CMAKE_SOURCE_DIR}/src)

# Build desktop application
add_subdirectory(src)

# Conditionally build Android server
if(BUILD_ANDROID_SERVER OR ANDROID)
    message(STATUS "Building Android server...")
    add_subdirectory(android_server)
endif()

enable_testing()

install(TARGETS MultiDeviceAndroidController
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY src/presentation/qml/
    DESTINATION share/qml
    FILES_MATCHING PATTERN "*.qml"
)

install(DIRECTORY resources/
    DESTINATION share/resources
    OPTIONAL
)

# Install Android server if built
if(BUILD_ANDROID_SERVER OR ANDROID)
    install(TARGETS android_server
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()
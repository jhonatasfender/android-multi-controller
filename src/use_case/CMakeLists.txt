# Use Case Layer - Business logic and use cases

set(USE_CASE_SOURCES
        device_management_use_case.cpp
        device_control_use_case.cpp
        automation_use_case.cpp
)

set(USE_CASE_HEADERS
        device_management_use_case.h
        device_control_use_case.h
        automation_use_case.h
)

add_library(use_case STATIC
        ${USE_CASE_SOURCES}
        ${USE_CASE_HEADERS}
)

target_include_directories(use_case PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(use_case
        core
        Qt6::Core
) 
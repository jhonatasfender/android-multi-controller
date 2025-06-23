add_subdirectory(logging)

set(CORE_SOURCES
        entities/device.cpp
        entities/device_status.cpp
        entities/command.cpp
        entities/automation_script.cpp
)

set(CORE_HEADERS
        entities/device.h
        entities/device_status.h
        entities/command.h
        entities/automation_script.h
        interfaces/device_repository.h
        interfaces/command_executor.h
)

add_library(core STATIC
        ${CORE_SOURCES}
        ${CORE_HEADERS}
)

target_include_directories(core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core
        Qt6::Core
        logging
) 
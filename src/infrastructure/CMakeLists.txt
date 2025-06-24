set(INFRASTRUCTURE_SOURCES
        adb/adb_service.cpp
        adb/adb_command_executor.cpp
        adb/screen_capture_service.cpp
        persistence/device_repository_impl.cpp
        persistence/adb_device_repository.cpp
        persistence/settings_manager.cpp
)

set(INFRASTRUCTURE_HEADERS
        adb/adb_service.h
        adb/adb_command_executor.h
        adb/screen_capture_service.h
        persistence/device_repository_impl.h
        persistence/adb_device_repository.h
        persistence/settings_manager.h
)

add_library(infrastructure STATIC
        ${INFRASTRUCTURE_SOURCES}
        ${INFRASTRUCTURE_HEADERS}
)

target_include_directories(infrastructure PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(infrastructure
        core
        use_case
        Qt6::Core
        Qt6::Gui
        Qt6::Network
) 
set(INFRASTRUCTURE_SOURCES
        adb/adb_service.cpp
        adb/adb_command_executor.cpp
        network/device_discovery_service.cpp
        network/network_device_repository.cpp
        network/native_protocol_client.cpp
        persistence/device_repository_impl.cpp
        persistence/adb_device_repository.cpp
        persistence/settings_manager.cpp
)

set(INFRASTRUCTURE_HEADERS
        adb/adb_service.h
        adb/adb_command_executor.h
        network/device_discovery_service.h
        network/network_device_repository.h
        network/native_protocol_client.h
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

add_subdirectory(streaming)

target_link_libraries(infrastructure
        core
        use_case
        streaming
        Qt6::Core
        Qt6::Gui
        Qt6::Network
) 
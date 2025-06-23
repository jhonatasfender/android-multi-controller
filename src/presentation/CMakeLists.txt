set(PRESENTATION_SOURCES
        main_window.cpp
        view_models/device_list_view_model.cpp
        view_models/device_control_view_model.cpp
        view_models/multi_device_mirror_view_model.cpp
        view_models/device_image_provider.cpp
)

set(PRESENTATION_HEADERS
        main_window.h
        view_models/device_list_view_model.h
        view_models/device_control_view_model.h
        view_models/multi_device_mirror_view_model.h
)

add_library(presentation STATIC
        ${PRESENTATION_SOURCES}
        ${PRESENTATION_HEADERS}
)

target_include_directories(presentation PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(presentation
        core
        use_case
        infrastructure
        Qt6::Core
        Qt6::Quick
        Qt6::Widgets
        Qt6::Network
) 
add_subdirectory(core)

add_subdirectory(use_case)

add_subdirectory(infrastructure)

add_subdirectory(presentation)

qt6_add_resources(QML_RESOURCES presentation/qml/qml.qrc)

add_executable(MultiDeviceAndroidController
        main.cpp
        ${QML_RESOURCES}
)

target_link_libraries(MultiDeviceAndroidController
        Qt6::Core
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Widgets
        Qt6::Network
        core
        use_case
        infrastructure
        presentation
)

set_target_properties(MultiDeviceAndroidController PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
)
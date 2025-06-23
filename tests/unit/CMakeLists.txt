find_package(GTest REQUIRED)

add_executable(test_core
    test_device.cpp
    test_device_status.cpp
)

target_link_libraries(test_core
    core
    GTest::gtest
    GTest::gtest_main
    Qt6::Core
)

add_executable(test_use_case
    test_device_management_use_case.cpp
)

target_link_libraries(test_use_case
    use_case
    core
    GTest::gtest
    GTest::gtest_main
    Qt6::Core
)

add_executable(test_infrastructure
    test_adb_service.cpp
)

target_link_libraries(test_infrastructure
    infrastructure
    core
    use_case
    GTest::gtest
    GTest::gtest_main
    Qt6::Core
    Qt6::Network
)

add_test(NAME CoreTests COMMAND test_core)
add_test(NAME UseCaseTests COMMAND test_use_case)
add_test(NAME InfrastructureTests COMMAND test_infrastructure)
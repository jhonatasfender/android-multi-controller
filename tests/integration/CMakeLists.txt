find_package(GTest REQUIRED)

add_executable(test_integration
    test_full_workflow.cpp
)

target_link_libraries(test_integration
    core
    use_case
    infrastructure
    presentation
    GTest::gtest
    GTest::gtest_main
    Qt6::Core
    Qt6::Quick
    Qt6::Widgets
    Qt6::Network
)

# Add tests to CTest
add_test(NAME IntegrationTests COMMAND test_integration) 
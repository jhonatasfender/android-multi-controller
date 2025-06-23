set(LOGGING_SOURCES
        logger.cpp
        logger.h
)

add_library(logging STATIC ${LOGGING_SOURCES})

target_link_libraries(logging
        Qt6::Core
)

target_include_directories(logging PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
) 
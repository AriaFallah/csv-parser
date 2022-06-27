project(AriaCsvParser)
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
include(GNUInstallDirs)

add_library(${PROJECT_NAME} INTERFACE)

set_target_properties(${PROJECT_NAME} PROPERTIES
    PUBLIC_HEADER parser.hpp
)

target_include_directories( ${PROJECT_NAME}
                INTERFACE
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<INSTALL_INTERFACE:include>
                )

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets)

install(EXPORT ${PROJECT_NAME}Targets
    NAMESPACE AriaCsvParser::
    FILE AriaCsvParserTargets.cmake
    DESTINATION lib/cmake/${PROJECT_NAME}
)
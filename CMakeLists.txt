project(AriaCsvParser)
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

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

configure_package_config_file(Config.cmake.in 
    ${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION "lib/cmake/${PROJECT_NAME}"
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    DESTINATION lib/cmake/${PROJECT_NAME}
    )
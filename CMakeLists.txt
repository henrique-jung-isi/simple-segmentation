cmake_minimum_required(VERSION 3.11)

project(simple_segmentation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(isisim_inference REQUIRED)
find_package(arg_parser QUIET)

if(NOT arg_parser_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        arg_parser
        GIT_REPOSITORY https://github.com/henrique-jung-isi/arg-parser.git
        GIT_TAG v1.0.0
    )
    FetchContent_MakeAvailable(arg_parser)
endif()

# ================== Sources ==================
# choose one:
add_executable(
    ${PROJECT_NAME}
    "src/SimpleSegmentation.cpp"
    "main.cpp"
)

# ================== Headers ==================
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)

# ================== Dependecies ==================
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
    arg_parser
    isisim_inference
)

# ================== Strip outputs if in release ==================
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s")
endif()

# Copy headers to build folder so the example works without installing
file(GLOB headers "${CMAKE_SOURCE_DIR}/include/*")
set(headersBuildLocation "${CMAKE_BINARY_DIR}/include/${PROJECT_NAME}")

file(COPY ${headers} DESTINATION ${headersBuildLocation})
add_custom_target(
    copy_headers
    ALL
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different ${headers} ${headersBuildLocation}
    COMMENT "Copying header files to build folder"
)

# ================== Installation directives ==================
install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include/${PROJECT_NAME})

# ================== find_package directives ==================
include(GNUInstallDirs)
set(INCLUDE_INSTALL_DIR
    ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    CACHE PATH
    "Location of header files"
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/install.cmake.in"
    "${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
    PATH_VARS INCLUDE_INSTALL_DIR
)

install(
    EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    DESTINATION lib/cmake/${PROJECT_NAME}
)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    DESTINATION lib/cmake/${PROJECT_NAME}
)

# ================== Uninstall target ==================
if(NOT TARGET uninstall)
    configure_file(
        "cmake/uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
        IMMEDIATE
        @ONLY
    )

    add_custom_target(
        uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake
    )
endif()

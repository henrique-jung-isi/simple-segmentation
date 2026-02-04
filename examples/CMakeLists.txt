cmake_minimum_required(VERSION 3.8)

project(example LANGUAGES CXX)

# find main package if the example project is being build independentle
if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    find_package(PROJECT_NAME_HERE REQUIRED)
endif()

add_executable(${PROJECT_NAME} example.cpp)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_BINARY_DIR}/include
)

target_link_libraries(
    ${PROJECT_NAME}
    # PROJECT_NAME_HERE
)

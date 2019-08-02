
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(dds-psm VERSION "1.0" LANGUAGES CXX)

set(SRCS src/main.cpp)

add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 11
)
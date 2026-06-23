cmake_minimum_required(VERSION 3.22)

project(fuzz_ReaderProxyData VERSION 1 LANGUAGES CXX)

# Find requirements
if(NOT fastcdr_FOUND)
    find_package(fastcdr 2 REQUIRED)
endif()

if(NOT foonathan_memory_FOUND)
    find_package(foonathan_memory REQUIRED)
endif()

if(NOT fastdds_FOUND)
    find_package(fastdds 3 REQUIRED)
endif()

message(STATUS "Configuring fuzz_ReaderProxyData...")
file(GLOB SOURCES_CXX "fuzz_*.cxx")

add_executable(fuzz_ReaderProxyData ${SOURCES_CXX})
target_include_directories(fuzz_ReaderProxyData PRIVATE ${CMAKE_SOURCE_DIR}/src/cpp)
target_link_libraries(fuzz_ReaderProxyData fastdds fastcdr foonathan_memory $ENV{LIB_FUZZING_ENGINE})

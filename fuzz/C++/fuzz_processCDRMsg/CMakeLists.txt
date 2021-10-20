cmake_minimum_required(VERSION 2.8.12)

if(NOT CMAKE_VERSION VERSION_LESS 3.0)
    cmake_policy(SET CMP0048 NEW)
endif()

project(fuzz_processCDRMsg)

# Find requirements
if(NOT fastcdr_FOUND)
    find_package(fastcdr REQUIRED)
endif()

if(NOT foonathan_memory_FOUND)
    find_package(foonathan_memory REQUIRED)
endif()

if(NOT fastrtps_FOUND)
    find_package(fastrtps REQUIRED)
endif()

message(STATUS "Configuring fuzz_processCDRMsg...")
file(GLOB SOURCES_CXX "fuzz_*.cxx")

add_executable(fuzz_processCDRMsg ${SOURCES_CXX})
target_link_libraries(fuzz_processCDRMsg fastrtps fastcdr foonathan_memory $ENV{LIB_FUZZING_ENGINE})
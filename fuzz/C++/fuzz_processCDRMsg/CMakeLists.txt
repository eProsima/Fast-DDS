cmake_minimum_required(VERSION 3.16.3)

project(fuzz_processCDRMsg VERSION 1 LANGUAGES CXX)

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

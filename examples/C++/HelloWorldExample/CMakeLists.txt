cmake_minimum_required(VERSION 2.8.12)
project("HelloWorldExample")

# Find requirements
find_package(Boost REQUIRED)
find_package(fastcdr REQUIRED)
find_package(fastrtps REQUIRED)

# Set C++11
include(CheckCXXCompilerFlag)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
        CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    check_cxx_compiler_flag(--std=c++11 SUPPORTS_CXX11)
    if(SUPPORTS_CXX11)
        add_compile_options(--std=c++11)
    else()
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()

message(STATUS "Configuring HelloWorld example...")
file(GLOB HELLOWORLD_EXAMPLE_SOURCES_CXX "*.cxx")
file(GLOB HELLOWORLD_EXAMPLE_SOURCES_CPP "*.cpp")

add_executable(HelloWorldExample ${HELLOWORLD_EXAMPLE_SOURCES_CXX} ${HELLOWORLD_EXAMPLE_SOURCES_CPP})
target_include_directories(HelloWorldExample PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(HelloWorldExample fastrtps fastcdr ${Boost_LIBRARIES})
install(TARGETS HelloWorldExample
    RUNTIME DESTINATION examples/C++/HelloWorldExample/${BIN_INSTALL_DIR})

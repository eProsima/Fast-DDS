cmake_minimum_required(VERSION 2.8.12)
project("DeadlineQoSExample")

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

message(STATUS "Configuring DeadlineQoS example...")
file(GLOB DEADLINEQOS_EXAMPLE_SOURCES "*.cxx")

add_executable(DeadlineQoSExample ${DEADLINEQOS_EXAMPLE_SOURCES})
target_include_directories(DeadlineQoSExample PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(DeadlineQoSExample fastrtps fastcdr ${Boost_LIBRARIES})
install(TARGETS DeadlineQoSExample
    RUNTIME DESTINATION examples/C++/DeadlineQoSExample/${BIN_INSTALL_DIR}
    )

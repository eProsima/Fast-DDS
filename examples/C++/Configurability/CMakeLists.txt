# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.8.12)
project("UseCaseDemonstrator")

# Find requirements
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

message(STATUS "Configuring UseCaseDemonstrator example...")
file(GLOB USECASEDEMONSTRATOR_EXAMPLE_SOURCES "*.cxx")

add_executable(UseCasePublisher ${USECASEDEMONSTRATOR_EXAMPLE_SOURCES} UseCasePublisher.cpp)
target_link_libraries(UseCasePublisher fastrtps fastcdr)
install(TARGETS UseCasePublisher
    RUNTIME DESTINATION examples/C++/UseCaseDemonstrator/${BIN_INSTALL_DIR}
    )

add_executable(UseCaseSubscriber ${USECASEDEMONSTRATOR_EXAMPLE_SOURCES} UseCaseSubscriber.cpp)
target_link_libraries(UseCaseSubscriber fastrtps fastcdr)
install(TARGETS UseCaseSubscriber
    RUNTIME DESTINATION examples/C++/UseCaseDemonstrator/${BIN_INSTALL_DIR}
    )

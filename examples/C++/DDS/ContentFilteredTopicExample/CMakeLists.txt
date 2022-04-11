# Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

cmake_minimum_required(VERSION 3.16.3)

project("ContentFilterTopic" VERSION 1 LANGUAGES CXX)

# Find requirements
if(NOT fastcdr_FOUND)
    find_package(fastcdr REQUIRED)
endif()

if(NOT fastrtps_FOUND)
    find_package(fastrtps REQUIRED)
endif()

message(STATUS "Configuring ContentFilterTopic examples...")

set(CFT_COMMON_SOURCES
    ContentFilteredTopicExample_main.cpp
    ContentFilteredTopicExamplePublisher.cpp
    ContentFilteredTopicExampleSubscriber.cpp
    HelloWorld.cxx
    HelloWorldPubSubTypes.cxx
    HelloWorldTypeObject.cxx
    )

add_executable(DDSContentFilteredTopicExample ${CFT_COMMON_SOURCES})
target_compile_definitions(DDSContentFilteredTopicExample PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
)
target_compile_features(DDSContentFilteredTopicExample PRIVATE cxx_std_11)
target_link_libraries(DDSContentFilteredTopicExample fastrtps fastcdr fastdds::optionparser)
install(TARGETS DDSContentFilteredTopicExample
    RUNTIME DESTINATION examples/C++/DDS/ContentFilteredTopicExample/${BIN_INSTALL_DIR})

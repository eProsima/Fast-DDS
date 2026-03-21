# Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

# Workaround for GCC 13 false positive with GoogleTest EXPECT_CALL matchers
# GCC 13 incorrectly reports uninitialized variables in GoogleTest's EXPECT_CALL
# when using matchers like _. This is fixed in GCC 14.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND 
   CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0 AND
   CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14.0)
    add_compile_options(-Wno-error=uninitialized)
    add_compile_options(-Wno-error=maybe-uninitialized)
    message(STATUS "Applied GCC 13 workaround to all unit tests")
endif()

add_compile_definitions(FASTDDS_NO_LIB)

add_subdirectory(cmake)
add_subdirectory(dds/collections)
add_subdirectory(dds/core)
add_subdirectory(dds/participant)
add_subdirectory(dds/publisher)
add_subdirectory(dds/status)
add_subdirectory(dds/subscriber)
add_subdirectory(dds/topic)
add_subdirectory(dds/topic/DDSSQLFilter)
add_subdirectory(dds/xtypes/serializers)
add_subdirectory(dds/xtypes/type_lookup_service)
add_subdirectory(dds/xtypes/type_representation)
add_subdirectory(fastdds/utils)
add_subdirectory(logging)
add_subdirectory(rtps/attributes)
add_subdirectory(rtps/builtin)
add_subdirectory(rtps/common)
add_subdirectory(rtps/DataSharing)
add_subdirectory(rtps/discovery)
add_subdirectory(rtps/domain)
if(NOT QNX)
    add_subdirectory(rtps/flowcontrol)
endif()
add_subdirectory(rtps/history)
add_subdirectory(rtps/network)
add_subdirectory(rtps/participant)
add_subdirectory(rtps/persistence)
add_subdirectory(rtps/reader)
add_subdirectory(rtps/resources/timedevent)
add_subdirectory(rtps/writer)
add_subdirectory(statistics/dds)
add_subdirectory(transport)
add_subdirectory(utils)
add_subdirectory(xmlparser)

if(SECURITY)
    add_subdirectory(security/authentication)
    add_subdirectory(security/cryptography)
    add_subdirectory(security/accesscontrol)
    add_subdirectory(security/logging)
    if(NOT QNX)
        add_subdirectory(rtps/security)
    endif()
endif()

if(FASTDDS_STATISTICS)
    add_subdirectory(statistics/rtps)
endif(FASTDDS_STATISTICS)

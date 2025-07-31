# Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

set(TYPE_ASSIGNABILITY_TESTS_SOURCE
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/exception/Exception.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/type_representation/TypeAssignability.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/type_representation/dds_xtypes_typeobjectPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/type_representation/TypeObjectRegistry.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/type_representation/TypeObjectUtils.cpp

    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp

    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp

    assignabilityPubSubTypes.cxx
    assignabilityTypeObjectSupport.cxx

    TypeAssignabilityTests.cpp
    )

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

add_executable(TypeAssignabilityTests ${TYPE_ASSIGNABILITY_TESTS_SOURCE})
target_compile_definitions(TypeAssignabilityTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(TypeAssignabilityTests PRIVATE
    ${PROJECT_SOURCE_DIR}/test/mock/dds/DomainParticipantFactory
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSDomainImpl

    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp

    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    )
target_link_libraries(TypeAssignabilityTests
    fastcdr
    fastdds::log
    fastdds::xtypes::dynamic-types::impl
    GTest::gtest
    ${CMAKE_DL_LIBS})
gtest_discover_tests(TypeAssignabilityTests SOURCES ${TYPEOBJECTUTILSTESTS_SOURCE})

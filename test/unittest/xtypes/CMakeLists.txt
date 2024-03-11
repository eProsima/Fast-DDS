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

if(WIN32)
    add_definitions(
        -D_WIN32_WINNT=0x0601
        -D_CRT_SECURE_NO_WARNINGS
        )
endif()

set(XTYPES_SOURCE
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/AnnotationDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/AnnotationParameterValue.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/BuiltinAnnotationsTypeObject.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicDataFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicDataPtr.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicPubSubType.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicType.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicTypeBuilder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicTypeBuilderFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicTypeBuilderPtr.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicTypeMember.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/DynamicTypePtr.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/MemberDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/TypeDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/TypeIdentifier.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/TypeIdentifierTypes.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/TypeNamesGenerator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/TypeObject.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/TypeObjectFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/TypeObjectHashId.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/TypesBase.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/Log.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/OStreamConsumer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutConsumer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutErrConsumer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/TopicDataType.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/string_convert.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    )

set(XTYPES_TEST_SOURCE
    XTypesTests.cpp
    idl/TypesTypeObject.cxx
    idl/Types.cxx
    idl/Typesv1.cxx
    idl/WideEnumTypeObject.cxx
    idl/WideEnum.cxx
    idl/WideEnumv1.cxx
    ${XTYPES_SOURCE}
    )

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND XTYPES_TEST_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

include_directories(mock/)

add_executable(XTypesTests ${XTYPES_TEST_SOURCE})
target_compile_definitions(XTypesTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(XTypesTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_libraries(XTypesTests GTest::gtest ${MOCKS})
if(MSVC OR MSVC_IDE)
    target_link_libraries(XTypesTests ${PRIVACY} fastcdr iphlpapi Shlwapi ws2_32)
else()
    target_link_libraries(XTypesTests ${PRIVACY} fastcdr)
endif()
gtest_discover_tests(XTypesTests)

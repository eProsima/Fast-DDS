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

if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER))
    include(${PROJECT_SOURCE_DIR}/cmake/common/gtest.cmake)
    check_gtest()

    if(GTEST_FOUND)
        if(WIN32)
            add_definitions(
                -D_WIN32_WINNT=0x0601
                -D_CRT_SECURE_NO_WARNINGS
                )
        endif()

        include_directories(${ASIO_INCLUDE_DIR})

        set(DYNAMIC_TYPES_SOURCE
            ${PROJECT_SOURCE_DIR}/src/cpp/types/AnnotationDescriptor.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/DynamicData.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/DynamicDataFactory.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/DynamicType.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/DynamicPubSubType.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/DynamicTypePtr.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/DynamicDataPtr.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/DynamicTypeBuilder.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/DynamicTypeBuilderPtr.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/DynamicTypeBuilderFactory.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/DynamicTypeMember.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/TypeDescriptor.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/MemberDescriptor.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/AnnotationParameterValue.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/TypeIdentifier.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/TypeIdentifierTypes.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/TypeObject.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/TypeObjectFactory.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/TypeObjectHashId.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/TypeNamesGenerator.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/TypesBase.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/types/BuiltinAnnotationsTypeObject.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/StdoutConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/FileConsumer.cpp
        )

        # External sources
        if(TINYXML2_SOURCE_DIR)
            list(APPEND DYNAMIC_TYPES_SOURCE
                ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
                )
        endif()

        set(DYNAMIC_TYPES_TEST_SOURCE
            DynamicTypesTests.cpp
            idl/Basic.cxx
            idl/BasicPubSubTypes.cxx
            idl/BasicTypeObject.cxx
            ${DYNAMIC_TYPES_SOURCE}

            ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLProfileManager.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParser.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLElementParser.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParserCommon.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/qos/QosPolicies.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/qos/ParameterTypes.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/qos/WriterQos.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/qos/ReaderQos.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/ThroughputControllerDescriptor.cpp
        )

        set(DYNAMIC_COMPLEX_TYPES_TEST_SOURCE
            DynamicComplexTypesTests.cpp
            idl/Test.cxx
            idl/TestPubSubTypes.cxx
            idl/TestTypeObject.cxx
            ${DYNAMIC_TYPES_SOURCE}
        )

        include_directories(mock/)

        add_executable(DynamicTypesTests ${DYNAMIC_TYPES_TEST_SOURCE})
        target_compile_definitions(DynamicTypesTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(DynamicTypesTests PRIVATE
            ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPTransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv4TransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv6TransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv4TransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv6TransportDescriptor
            ${TINYXML2_INCLUDE_DIR}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
            )
        target_link_libraries(DynamicTypesTests ${GTEST_LIBRARIES}
            $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
            $<$<BOOL:${WIN32}>:ws2_32>
            ${TINYXML2_LIBRARY}
            fastcdr
        )
        add_gtest(DynamicTypesTests SOURCES ${DYNAMIC_TYPES_TEST_SOURCE})


        add_executable(DynamicComplexTypesTests ${DYNAMIC_COMPLEX_TYPES_TEST_SOURCE})
        target_compile_definitions(DynamicComplexTypesTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(DynamicComplexTypesTests PRIVATE
            ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
        target_link_libraries(DynamicComplexTypesTests ${GTEST_LIBRARIES}
            $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
            $<$<BOOL:${WIN32}>:ws2_32>
            ${TINYXML2_LIBRARY}
            fastcdr
        )
        add_gtest(DynamicComplexTypesTests SOURCES ${DYNAMIC_COMPLEX_TYPES_TEST_SOURCE})

    endif()
endif()

configure_file("types.xml" "types.xml" COPYONLY)

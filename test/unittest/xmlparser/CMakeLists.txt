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
    check_gmock()

    if(GTEST_FOUND AND GMOCK_FOUND)
        if(WIN32)
            add_definitions(
                -D_WIN32_WINNT=0x0601
                -D_CRT_SECURE_NO_WARNINGS
                )
        endif()

        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_xml_profiles.xml
            ${CMAKE_CURRENT_BINARY_DIR}/test_xml_profiles.xml
            COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_xml_security_profiles.xml
            ${CMAKE_CURRENT_BINARY_DIR}/test_xml_security_profiles.xml
            COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_xml_profiles_rooted.xml
            ${CMAKE_CURRENT_BINARY_DIR}/test_xml_profiles_rooted.xml
            COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/log_def_file.xml
            ${CMAKE_CURRENT_BINARY_DIR}/log_def_file.xml
            COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/log_inactive.xml
            ${CMAKE_CURRENT_BINARY_DIR}/log_inactive.xml
            COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/log_node_file_append.xml
            ${CMAKE_CURRENT_BINARY_DIR}/log_node_file_append.xml
            COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tls_config.xml
            ${CMAKE_CURRENT_BINARY_DIR}/tls_config.xml
            COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/UDP_transport_descriptors_config.xml
            ${CMAKE_CURRENT_BINARY_DIR}/UDP_transport_descriptors_config.xml
            COPYONLY)

        set(XMLPROFILEPARSER_SOURCE
            XMLProfileParserTests.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/xmlparser/XMLProfileManager.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/xmlparser/XMLParser.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/xmlparser/XMLElementParser.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/xmlparser/XMLParserCommon.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/qos/QosPolicies.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/qos/ParameterTypes.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/qos/WriterQos.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/qos/ReaderQos.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/rtps/flowcontrol/ThroughputControllerDescriptor.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/utils/IPFinder.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/utils/IPLocator.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/AnnotationDescriptor.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicData.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicDataFactory.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicType.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicPubSubType.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicTypePtr.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicDataPtr.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicTypeBuilder.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicTypeBuilderPtr.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicTypeBuilderFactory.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicTypeMember.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/TypeDescriptor.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/MemberDescriptor.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/AnnotationParameterValue.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/TypeIdentifier.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/TypeIdentifierTypes.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/TypeObject.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/TypeObjectFactory.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/TypeObjectHashId.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/TypeNamesGenerator.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/TypesBase.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/BuiltinAnnotationsTypeObject.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/utils/md5.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/rtps/common/Time_t.cpp
            )

        # External sources
        if(TINYXML2_SOURCE_DIR)
            list(APPEND XMLPROFILEPARSER_SOURCE
                ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
                )
        endif()

        include_directories(${TINYXML2_INCLUDE_DIR})

        add_executable(XMLProfileParserTests ${XMLPROFILEPARSER_SOURCE})
        target_compile_definitions(XMLProfileParserTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(XMLProfileParserTests PRIVATE
            ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/Log
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPTransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv4TransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv6TransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPTransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv4TransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv6TransportDescriptor
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)

        target_link_libraries(XMLProfileParserTests ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES}
            $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
            $<$<BOOL:${WIN32}>:ws2_32>
            ${TINYXML2_LIBRARY}
            fastcdr
            )
        add_gtest(XMLProfileParserTests SOURCES XMLProfileParserTests.cpp)


        set(XMLPARSER_SOURCE
            XMLParserTests.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/xmlparser/XMLProfileManager.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/xmlparser/XMLParser.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/xmlparser/XMLElementParser.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/xmlparser/XMLParserCommon.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/qos/QosPolicies.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/qos/ParameterTypes.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/qos/WriterQos.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/qos/ReaderQos.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/rtps/flowcontrol/ThroughputControllerDescriptor.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/log/StdoutConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/log/FileConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/utils/IPFinder.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/utils/IPLocator.cpp

            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/AnnotationDescriptor.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicData.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicDataFactory.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicType.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicPubSubType.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicTypePtr.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicDataPtr.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicTypeBuilder.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicTypeBuilderPtr.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicTypeBuilderFactory.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/DynamicTypeMember.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/TypeDescriptor.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/MemberDescriptor.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/AnnotationParameterValue.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/TypeIdentifier.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/TypeIdentifierTypes.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/TypeObject.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/TypeObjectFactory.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/TypeObjectHashId.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/TypeNamesGenerator.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/TypesBase.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/types/BuiltinAnnotationsTypeObject.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/utils/md5.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps/rtps/common/Time_t.cpp
        )

        # External sources
        if(TINYXML2_SOURCE_DIR)
            list(APPEND XMLPARSER_SOURCE
                ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
                )
        endif()

        include_directories(${TINYXML2_INCLUDE_DIR})

        add_executable(XMLParserTests ${XMLPARSER_SOURCE})
        target_compile_definitions(XMLParserTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(XMLParserTests PRIVATE
            ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPTransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv4TransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv6TransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPTransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv4TransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv6TransportDescriptor
            ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_BINARY_DIR}/include
            )

        target_link_libraries(XMLParserTests ${GTEST_LIBRARIES}
            $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
            $<$<BOOL:${WIN32}>:ws2_32>
            ${TINYXML2_LIBRARY}
            fastcdr
        )
        add_gtest(XMLParserTests SOURCES XMLParserTests.cpp)
    endif()
endif()

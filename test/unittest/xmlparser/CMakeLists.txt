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

        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_xml_profiles.xml
            ${CMAKE_CURRENT_BINARY_DIR}/test_xml_profiles.xml
            COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_xml_security_profiles.xml
            ${CMAKE_CURRENT_BINARY_DIR}/test_xml_security_profiles.xml
            COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_xml_profiles_rooted.xml
            ${CMAKE_CURRENT_BINARY_DIR}/test_xml_profiles_rooted.xml
            COPYONLY)

        set(XMLPROFILEPARSER_SOURCE
            XMLProfileParserTests.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLProfileManager.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParser.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLElementParser.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParserCommon.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/qos/QosPolicies.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/qos/ParameterTypes.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/qos/WriterQos.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/qos/ReaderQos.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/ThroughputControllerDescriptor.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/StdoutConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp

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
            ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
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
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPTransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv4TransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv6TransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv4TransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv6TransportDescriptor
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)

        if(MSVC OR MSVC_IDE)
            target_link_libraries(XMLProfileParserTests ${PRIVACY} fastcdr)
        else()
            target_link_libraries(XMLProfileParserTests ${PRIVACY} fastcdr)
        endif()

        target_link_libraries(XMLProfileParserTests ${GTEST_LIBRARIES})
        if(MSVC OR MSVC_IDE)
            target_link_libraries(XMLProfileParserTests ${PRIVACY} iphlpapi Shlwapi ws2_32)
        endif()
        if(TINYXML2_LIBRARY)
            target_link_libraries(XMLProfileParserTests ${PRIVACY}
                ${TINYXML2_LIBRARY}
                )
        endif()
        add_gtest(XMLProfileParserTests SOURCES XMLProfileParserTests.cpp)


        set(XMLPARSER_SOURCE
            XMLParserTests.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLProfileManager.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParser.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLElementParser.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParserCommon.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/qos/QosPolicies.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/qos/ParameterTypes.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/qos/WriterQos.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/qos/ReaderQos.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/ThroughputControllerDescriptor.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/StdoutConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp

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
            ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
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
            ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPTransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv4TransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv6TransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv4TransportDescriptor
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv6TransportDescriptor
            ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_BINARY_DIR}/include
            )
            
        if(MSVC OR MSVC_IDE)
            target_link_libraries(XMLParserTests ${PRIVACY} iphlpapi Shlwapi ws2_32)
        endif()

        if(MSVC OR MSVC_IDE)
            target_link_libraries(XMLParserTests ${PRIVACY} fastcdr)
        else()
            target_link_libraries(XMLParserTests ${PRIVACY} fastcdr)
        endif()

        target_link_libraries(XMLParserTests ${GTEST_LIBRARIES} )
        if(TINYXML2_LIBRARY)
            target_link_libraries(XMLParserTests ${PRIVACY} ${TINYXML2_LIBRARY} )
        endif()
        add_gtest(XMLParserTests SOURCES XMLParserTests.cpp)
    endif()
endif()


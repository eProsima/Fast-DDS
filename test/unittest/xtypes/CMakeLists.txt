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

        set(XTYPES_SOURCE
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
            ${PROJECT_SOURCE_DIR}/src/cpp/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/StdoutConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/qos/QosPolicies.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/qos/ParameterTypes.cpp
        )

        set(XTYPES_TEST_SOURCE
            XTypesTests.cpp
            idl/TypesTypeObject.cxx
            idl/Types.cxx
            idl/WideEnumTypeObject.cxx
            idl/WideEnum.cxx
            ${XTYPES_SOURCE}
        )

        include_directories(mock/)

        add_executable(XTypesTests ${XTYPES_TEST_SOURCE})
        target_compile_definitions(XTypesTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(XTypesTests PRIVATE
            ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
            #${PROJECT_SOURCE_DIR}/test/mock/rtps/XTypes
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
            )
        target_link_libraries(XTypesTests ${GTEST_LIBRARIES} ${MOCKS})
        if(MSVC OR MSVC_IDE)
            target_link_libraries(XTypesTests ${PRIVACY} fastcdr iphlpapi Shlwapi ws2_32)
        else()
            target_link_libraries(XTypesTests ${PRIVACY} fastcdr)
        endif()
        add_gtest(XTypesTests SOURCES ${XTYPES_TEST_SOURCE})

    endif()
endif()

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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../profiles/test_xml_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/test_xml_profile.xml
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../profiles/test_xml_for_string_profile.xml
        ${CMAKE_CURRENT_BINARY_DIR}/test_xml_for_string_profile.xml
        COPYONLY)

set(DATAWRITERTESTS_SOURCE DataWriterTests.cpp)

set(PUBLISHERTESTS_SOURCE PublisherTests.cpp)

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND PUBLISHERTESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
        list(APPEND DATAWRITERTESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

add_executable(PublisherTests ${PUBLISHERTESTS_SOURCE})
target_include_directories(PublisherTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    ${PROJECT_SOURCE_DIR}/test/utils
    )
target_link_libraries(PublisherTests fastdds fastcdr foonathan_memory
    GTest::gmock
    $<$<BOOL:${QNX}>:socket>
    ${CMAKE_DL_LIBS})
gtest_discover_tests(PublisherTests)

get_property(fastdds_include_directories TARGET ${PROJECT_NAME} PROPERTY INCLUDE_DIRECTORIES)

add_executable(DataWriterTests ${DATAWRITERTESTS_SOURCE})
target_link_units(DataWriterTests UNITS core MOCKS DataWriterHistory)
target_include_directories(DataWriterTests PRIVATE
    ${fastdds_include_directories}
    )
target_link_libraries(DataWriterTests
    GTest::gmock
    )
gtest_discover_tests(DataWriterTests
    PROPERTIES ENVIRONMENT "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")

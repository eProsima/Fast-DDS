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

# Only include tinyxml2 headers manually if the associated config predates the use of CMake targets
if(TINYXML2_INCLUDE_DIR)
    include_directories(${TINYXML2_INCLUDE_DIR})
endif(TINYXML2_INCLUDE_DIR)

add_executable(AccessControlTests ${CMAKE_CURRENT_SOURCE_DIR}/AccessControlTests.cpp)

target_include_directories(AccessControlTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )

target_link_units(AccessControlTests UNITS rtps::proxy-data security::authentication security::access-control MOCKS
    MessageReceiver rtps::network-factory::NetworkFactory RTPSDomainImpl xtypes::TypeObjectUtils)
target_link_libraries(AccessControlTests
    GTest::gmock
    ${OPENSSL_LIBRARIES}
    fastcdr
    foonathan_memory
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    $<$<BOOL:${QNX}>:socket>
    ${TINYXML2_LIBRARY}
    $<$<BOOL:${LibP11_FOUND}>:eProsima_p11>  # $<TARGET_NAME_IF_EXISTS:eProsima_p11>
    )

gtest_discover_tests(AccessControlTests
    PROPERTIES ENVIRONMENT "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")

####################################################################################################
####################################################################################################
# DistinguishedNameTests
set(
    DISTINGUISHEDNAME_TEST_NAME
    "DistinguishedNameTests"
)

add_executable(
    ${DISTINGUISHEDNAME_TEST_NAME}

    # test files
    ${CMAKE_CURRENT_SOURCE_DIR}/DistinguishedNameTests.cpp
)

target_include_directories(
    ${DISTINGUISHEDNAME_TEST_NAME}
    PRIVATE
        ${Asio_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/cpp
)

target_link_units(${DISTINGUISHEDNAME_TEST_NAME} UNITS security::common)
target_link_libraries(${DISTINGUISHEDNAME_TEST_NAME}
    fastcdr
    GTest::gtest
)

gtest_discover_tests( ${DISTINGUISHEDNAME_TEST_NAME})

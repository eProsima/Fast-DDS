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

add_executable(SecurityAuthentication
    ${CMAKE_CURRENT_SOURCE_DIR}/SecurityTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SecurityInitializationTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SecurityValidationRemoteTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SecurityHandshakeProcessTests.cpp)

if(WIN32)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/SecurityInitializationTests.cpp PROPERTIES COMPILE_OPTIONS /bigobj)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/SecurityHandshakeProcessTests.cpp PROPERTIES COMPILE_OPTIONS /bigobj)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/SecurityValidationRemoteTests.cpp PROPERTIES COMPILE_OPTIONS /bigobj)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/SecurityInitializationTests.cpp PROPERTIES COMPILE_OPTIONS /bigobj)
endif()

target_include_directories(SecurityAuthentication PRIVATE
    ${Asio_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_units(SecurityAuthentication UNITS ${STATISTICS_TYPES_UNIT} rtps::proxy-data rtps::history rtps::security-logging
    security::common dds::participant-qos rtps::locator-selector-sender rtps::flow-control
    rtps::security-manager MOCKS BaseReader StatefulReader StatelessReader StatelessWriter BaseWriter StatefulWriter
    EDP MessageReceiver rtps::network-factory::NetworkFactory PDP DomainParticipantFactory RTPSMessageGroup
    RTPSDomainImpl RTPSParticipantImpl SecurityPluginFactory ReaderHistory WriterHistory WLP xtypes::TypeObjectUtils)
target_link_libraries(SecurityAuthentication
    fastcdr
    foonathan_memory
    GTest::gmock
    ${OPENSSL_LIBRARIES}
    )
gtest_discover_tests(SecurityAuthentication)

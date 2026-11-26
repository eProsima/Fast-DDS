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

add_executable(BuiltinPKIDH ${CMAKE_CURRENT_SOURCE_DIR}/BuiltinPKIDHTests.cpp)
target_include_directories(BuiltinPKIDH PRIVATE
    ${Asio_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_units(BuiltinPKIDH UNITS rtps::proxy-data security::authentication MOCKS MessageReceiver
    rtps::network-factory::NetworkFactory RTPSDomainImpl xtypes::TypeObjectUtils)
target_link_libraries(BuiltinPKIDH
    GTest::gtest
    ${OPENSSL_LIBRARIES}
    fastcdr
    foonathan_memory
    $<$<BOOL:${WIN32}>:ws2_32>
    $<$<BOOL:${QNX}>:socket>
    $<$<BOOL:${LibP11_FOUND}>:eProsima_p11>  # $<TARGET_NAME_IF_EXISTS:eProsima_p11>
    )
gtest_discover_tests(BuiltinPKIDH
    PROPERTIES ENVIRONMENT "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs")

# Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

###################################  Prepare test files  ####################################################
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/regressions" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")


##############################  BuiltinDataSerializationTests  ##############################################
set(BUILTIN_DATA_SERIALIZATION_TESTS_SOURCE BuiltinDataSerializationTests.cpp
    )

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND BUILTIN_DATA_SERIALIZATION_TESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()


add_executable(BuiltinDataSerializationTests ${BUILTIN_DATA_SERIALIZATION_TESTS_SOURCE})
target_link_units(BuiltinDataSerializationTests UNITS rtps::proxy-data MOCKS MessageReceiver
    rtps::network-factory::NetworkFactory RTPSDomainImpl xtypes::TypeObjectUtils)
target_include_directories(BuiltinDataSerializationTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${Asio_INCLUDE_DIR}
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )
target_link_libraries(BuiltinDataSerializationTests
    fastcdr
    foonathan_memory
    GTest::gmock
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    $<$<BOOL:${QNX}>:socket>
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto$<$<BOOL:${WIN32}>:$<SEMICOLON>crypt32.lib>>
    ${CMAKE_DL_LIBS})

gtest_discover_tests(BuiltinDataSerializationTests)

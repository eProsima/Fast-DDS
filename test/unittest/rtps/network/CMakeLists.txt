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

set(NETWORKFACTORYTESTS_SOURCE
    NetworkFactoryTests.cpp
    mock/MockTransport.cpp
    )

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND NETWORKFACTORYTESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

include_directories(mock/)

add_executable(NetworkFactoryTests ${NETWORKFACTORYTESTS_SOURCE})
target_include_directories(NetworkFactoryTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )
target_link_units(NetworkFactoryTests UNITS rtps::network-factory rtps::timed-event rtps::parameter-list rtps::udp-transport MOCKS MessageReceiver)
target_link_libraries(NetworkFactoryTests
    fastcdr
    foonathan_memory
    GTest::gtest ${MOCKS}
    $<$<BOOL:${TLS_FOUND}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
    $<$<BOOL:${QNX}>:socket>
    ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

add_executable(ExternalLocatorsTests
    ExternalLocatorsTests.cpp
    )
target_link_units(ExternalLocatorsTests UNITS rtps::external-locators MOCKS xtypes::TypeObjectUtils
    MessageReceiver RTPSDomainImpl)
target_include_directories(ExternalLocatorsTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    )
target_link_libraries(ExternalLocatorsTests
    fastcdr
    foonathan_memory
    GTest::gmock
    $<$<BOOL:${QNX}>:socket>
    ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

if(WIN32)
    target_link_libraries(NetworkFactoryTests IPHLPAPI shlwapi) # Later so mocks have precedence
    target_link_libraries(ExternalLocatorsTests IPHLPAPI shlwapi) # Later so mocks have precedence
endif()

gtest_discover_tests(NetworkFactoryTests)
gtest_discover_tests(ExternalLocatorsTests)

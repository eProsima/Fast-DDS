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

        set(UDPV4TESTS_SOURCE
            UDPv4Tests.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/StdoutConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/transport/UDPv4Transport.cpp)

        set(UDPV6TESTS_SOURCE
            UDPv6Tests.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/StdoutConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/transport/UDPv6Transport.cpp)

        set(TEST_UDPV4TESTS_SOURCE
            test_UDPv4Tests.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/RTPSMessageCreator.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/CDRMessagePool.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/StdoutConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/qos/ParameterList.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/qos/QosPolicies.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/qos/ParameterTypes.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/utils/eClock.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/transport/test_UDPv4Transport.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/transport/UDPv4Transport.cpp)

        include_directories(mock/)

        add_executable(UDPv4Tests ${UDPV4TESTS_SOURCE})
        target_compile_definitions(UDPv4Tests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(UDPv4Tests PRIVATE ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
        target_link_libraries(UDPv4Tests ${GTEST_LIBRARIES} ${MOCKS} )
        if(MSVC OR MSVC_IDE)
            target_link_libraries(UDPv4Tests ${PRIVACY}
                iphlpapi Shlwapi
                )
        endif()
        add_gtest(UDPv4Tests SOURCES ${UDPV4TESTS_SOURCE})

        option(DISABLE_UDPV6_TESTS "Disable UDPv6 tests because fails in some systems" OFF)

        if(NOT DISABLE_UDPV6_TESTS)
            add_executable(UDPv6Tests ${UDPV6TESTS_SOURCE})
            target_compile_definitions(UDPv6Tests PRIVATE FASTRTPS_NO_LIB)
            target_include_directories(UDPv6Tests PRIVATE ${GTEST_INCLUDE_DIRS}
                ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
            target_link_libraries(UDPv6Tests ${GTEST_LIBRARIES} ${MOCKS} )
            if(MSVC OR MSVC_IDE)
                target_link_libraries(UDPv6Tests ${PRIVACY}
                    iphlpapi Shlwapi
                    )
            endif()
            add_gtest(UDPv6Tests SOURCES ${UDPV6TESTS_SOURCE})
        endif()

        add_executable(test_UDPv4Tests ${TEST_UDPV4TESTS_SOURCE})
        target_compile_definitions(test_UDPv4Tests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(test_UDPv4Tests PRIVATE ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
        target_link_libraries(test_UDPv4Tests ${GTEST_LIBRARIES} ${MOCKS} )
        if(MSVC OR MSVC_IDE)
            target_link_libraries(test_UDPv4Tests ${PRIVACY}
                iphlpapi Shlwapi
                )
        endif()
        add_gtest(test_UDPv4Tests SOURCES ${TEST_UDPV4TESTS_SOURCE})
    endif()
endif()

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
        find_package(Threads REQUIRED)

        set(NETWORKFACTORYTESTS_SOURCE
            NetworkFactoryTests.cpp
            mock/MockTransport.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/NetworkFactory.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/StdoutConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
        )

        add_executable(NetworkFactoryTests ${NETWORKFACTORYTESTS_SOURCE})
        set_common_compile_options(NetworkFactoryTests)
        target_compile_definitions(NetworkFactoryTests
            PRIVATE
                FASTRTPS_NO_LIB
                $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0x0601>
            )
        target_include_directories(NetworkFactoryTests
            PRIVATE
                mock/
                ${GTEST_INCLUDE_DIRS}
                ${GMOCK_INCLUDE_DIRS}
                ${PROJECT_SOURCE_DIR}/test/mock/rtps/ReceiverResource
                ${PROJECT_SOURCE_DIR}/include
                ${PROJECT_BINARY_DIR}/include
                ${PROJECT_SOURCE_DIR}/src/cpp
                ${ASIO_INCLUDE_DIR}
            )
        target_link_libraries(NetworkFactoryTests
            PRIVATE
                ${GTEST_LIBRARIES}
                ${MOCKS}
                ${CMAKE_THREAD_LIBS_INIT}
                ${CMAKE_DL_LIBS}
                $<$<PLATFORM_ID:Windows>:iphlpapi>
                $<$<PLATFORM_ID:Windows>:Shlwapi>
            )
        add_gtest(NetworkFactoryTests SOURCES ${NETWORKFACTORYTESTS_SOURCE})

    endif()
endif()

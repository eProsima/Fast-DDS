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

        set(TIMEDEVENTTESTS_SOURCE
            mock/MockEvent.cpp
            mock/MockParentEvent.cpp
            TimedEventTests.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEventImpl.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/TimedEvent.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
            )

        add_executable(TimedEventTests ${TIMEDEVENTTESTS_SOURCE})
        set_common_compile_options(TimedEventTests)
        target_compile_definitions(TimedEventTests
            PRIVATE
                FASTRTPS_NO_LIB
                $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0x0601>
            )
        target_include_directories(TimedEventTests
            PRIVATE
                ${GTEST_INCLUDE_DIRS}
                ${PROJECT_SOURCE_DIR}/include
                ${PROJECT_BINARY_DIR}/include
                ${ASIO_INCLUDE_DIR}
            )
        target_link_libraries(TimedEventTests
            PRIVATE
                ${GTEST_LIBRARIES}
                ${CMAKE_THREAD_LIBS_INIT}
                ${CMAKE_DL_LIBS}
            )
        add_gtest(TimedEventTests SOURCES ${TIMEDEVENTTESTS_SOURCE})
    endif()
endif()

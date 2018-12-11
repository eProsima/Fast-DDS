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
            add_definitions(-D_WIN32_WINNT=0x0601)
        endif()

        set(LOGTESTS_SOURCE
            ${PROJECT_SOURCE_DIR}/src/cpp/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/StdoutConsumer.cpp
            LogTests.cpp)

        include_directories(mock/)

        add_executable(LogTests ${LOGTESTS_SOURCE})
        target_compile_definitions(LogTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(LogTests PRIVATE ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
        target_link_libraries(LogTests ${GTEST_LIBRARIES} ${MOCKS} )
        if(MSVC OR MSVC_IDE)
            target_link_libraries(LogTests ${PRIVACY}
                iphlpapi Shlwapi
                )
        endif()
        add_gtest(LogTests SOURCES ${LOGTESTS_SOURCE})

        set(LOGFILETESTS_SOURCE
            ${PROJECT_SOURCE_DIR}/src/cpp/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/StdoutConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/FileConsumer.cpp
            LogFileTests.cpp)

        # External sources
        if(TINYXML2_SOURCE_DIR)
            list(APPEND LOGFILETESTS_SOURCE
                ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
                )
        endif()

        include_directories(${TINYXML2_INCLUDE_DIR})

        add_executable(LogFileTests ${LOGFILETESTS_SOURCE})
        target_compile_definitions(LogFileTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(LogFileTests PRIVATE ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
        target_link_libraries(LogFileTests ${GTEST_LIBRARIES} ${MOCKS} )
        if(MSVC OR MSVC_IDE)
            target_link_libraries(LogFileTests ${PRIVACY}
                iphlpapi Shlwapi
                )
        endif()
        target_link_libraries(LogFileTests ${PRIVACY} fastcdr)
        if(TINYXML2_LIBRARY)
            target_link_libraries(LogFileTests ${PRIVACY} ${TINYXML2_LIBRARY} )
        endif()
        add_gtest(LogFileTests SOURCES ${LOGFILETESTS_SOURCE})

    endif()
endif()

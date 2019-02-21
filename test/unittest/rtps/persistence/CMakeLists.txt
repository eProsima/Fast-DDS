# Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
        set(PERSISTENCETESTS_SOURCE
            PersistenceTests.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/PersistenceFactory.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/SQLite3PersistenceService.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/sqlite3.c
            ${PROJECT_SOURCE_DIR}/src/cpp/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/log/StdoutConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/CacheChangePool.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp)

        add_executable(PersistenceTests ${PERSISTENCETESTS_SOURCE})
        target_compile_definitions(PersistenceTests PRIVATE FASTRTPS_NO_LIB)
        target_include_directories(PersistenceTests PRIVATE ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/cpp
            )
        target_link_libraries(PersistenceTests foonathan_memory ${GTEST_LIBRARIES} ${CMAKE_DL_LIBS})
        if(MSVC OR MSVC_IDE)
            target_link_libraries(PersistenceTests ${PRIVACY}
                iphlpapi Shlwapi
                )
        endif()
        add_gtest(PersistenceTests SOURCES ${PERSISTENCETESTS_SOURCE})
    endif()
endif()

# Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

set(DDSSQLFILTERTEST_SOURCES DDSSQLFilterTests.cpp)
file(GLOB DDSSQLFILTER_TYPE_SOURCES data_types/*.cxx)

set(DDSSQLFILTER_SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/DDSFilterFactory.cpp
    )

file(GLOB DDSSQLFILTER_LIB_SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/string_convert.cpp
    )

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

add_executable(DDSSQLFilterTests
    ${DDSSQLFILTERTEST_SOURCES}
    ${DDSSQLFILTER_TYPE_SOURCES}
    ${DDSSQLFILTER_SOURCES}
    ${DDSSQLFILTER_LIB_SOURCES}
    )
target_compile_definitions(DDSSQLFilterTests PRIVATE FASTRTPS_NO_LIB
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(DDSSQLFilterTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_libraries(DDSSQLFilterTests fastcdr foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS})
add_gtest(DDSSQLFilterTests SOURCES ${DDSSQLFILTERTEST_SOURCES})

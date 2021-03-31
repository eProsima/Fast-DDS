# Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

    if (GTEST_FOUND)
        if(WIN32)
            add_definitions(-D_WIN32_WINNT=0x0601)
        endif()

        set(STATISTICS_DOMAINPARTICIPANT_TESTS_SOURCE
            StatisticsDomainParticipantTests.cpp
			${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/typesPubSubTypes.cxx
			${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/types.cxx)
        set(STATISTICS_QOS_TESTS_SOURCE
            StatisticsQosTests.cpp)

        add_executable(StatisticsDomainParticipantTests ${STATISTICS_DOMAINPARTICIPANT_TESTS_SOURCE})
        target_compile_definitions(StatisticsDomainParticipantTests PRIVATE FASTRTPS_NO_LIB
            $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
            $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNAL_DEBUG> # Internal debug activated.
            )
        target_include_directories(StatisticsDomainParticipantTests PRIVATE ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include ${PROJECT_SOURCE_DIR}/src/cpp)
        target_link_libraries(StatisticsDomainParticipantTests fastrtps fastcdr ${GTEST_LIBRARIES})
        add_gtest(StatisticsDomainParticipantTests SOURCES ${STATISTICS_DOMAINPARTICIPANT_TESTS_SOURCE})

        add_executable(StatisticsQosTests ${STATISTICS_QOS_TESTS_SOURCE})
        target_compile_definitions(StatisticsQosTests PRIVATE FASTRTPS_NO_LIB
            $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
            $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNAL_DEBUG> # Internal debug activated.
            )
        target_include_directories(StatisticsQosTests PRIVATE ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include ${PROJECT_SOURCE_DIR}/src/cpp)
        target_link_libraries(StatisticsQosTests fastrtps ${GTEST_LIBRARIES})
        add_gtest(StatisticsQosTests SOURCES ${STATISTICS_QOS_TESTS_SOURCE})
    endif()
endif()

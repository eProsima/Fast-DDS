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
    check_gmock()

    set(LOG_SOURCES
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/Log.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/OStreamConsumer.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutConsumer.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutErrConsumer.cpp
    )

    if(GTEST_FOUND)
        if(WIN32)
            add_definitions(-D_WIN32_WINNT=0x0601)
        endif()

        set(CONDITION_TESTS_SOURCE
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/Condition.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/ConditionNotifier.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/GuardCondition.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/StatusCondition.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/StatusConditionImpl.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/WaitSet.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/WaitSetImpl.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
            ${LOG_SOURCES}
            ConditionTests.cpp)

        add_executable(ConditionTests ${CONDITION_TESTS_SOURCE})
        target_compile_definitions(ConditionTests PRIVATE FASTRTPS_NO_LIB
            $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
            $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
            )
        target_include_directories(ConditionTests PRIVATE ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_BINARY_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/cpp
            )
        target_link_libraries(ConditionTests ${GTEST_LIBRARIES})
        add_gtest(ConditionTests SOURCES ${CONDITION_TESTS_SOURCE})
    endif()

    if(GMOCK_FOUND)
        ### ConditionNotifier ###
        set(CONDITION_NOTIFIER_TESTS_SOURCE
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/Condition.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/ConditionNotifier.cpp
            ${LOG_SOURCES}
            ConditionNotifierTests.cpp)

        add_executable(ConditionNotifierTests ${CONDITION_NOTIFIER_TESTS_SOURCE})
        target_compile_definitions(ConditionNotifierTests PRIVATE FASTRTPS_NO_LIB
            $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
            $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
            )
        target_include_directories(ConditionNotifierTests PRIVATE
            mock/WaitSetImpl
            ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_BINARY_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/cpp
            )
        target_link_libraries(ConditionNotifierTests ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES})
        add_gtest(ConditionNotifierTests SOURCES ${CONDITION_NOTIFIER_TESTS_SOURCE})

        ### StatusConditionImpl ###
        set(STATUS_CONDITION_IMPL_TESTS_SOURCE
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/Condition.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/StatusCondition.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/StatusConditionImpl.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
            ${LOG_SOURCES}
            StatusConditionImplTests.cpp)

        add_executable(StatusConditionImplTests ${STATUS_CONDITION_IMPL_TESTS_SOURCE})
        target_compile_definitions(StatusConditionImplTests PRIVATE FASTRTPS_NO_LIB
            $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
            $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
            )
        target_include_directories(StatusConditionImplTests PRIVATE
            mock/ConditionNotifier
            ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_BINARY_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/cpp
            )
        target_link_libraries(StatusConditionImplTests ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES})
        add_gtest(StatusConditionImplTests SOURCES ${STATUS_CONDITION_IMPL_TESTS_SOURCE})

        ### WaitSetImpl ###
        set(WAITSET_IMPL_TESTS_SOURCE
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/Condition.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/WaitSetImpl.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
            ${LOG_SOURCES}
            WaitSetImplTests.cpp)

        add_executable(WaitSetImplTests ${WAITSET_IMPL_TESTS_SOURCE})
        target_compile_definitions(WaitSetImplTests PRIVATE FASTRTPS_NO_LIB
            $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
            $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
            )
        target_include_directories(WaitSetImplTests PRIVATE
            mock/ConditionNotifier
            ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_BINARY_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/cpp
            )
        target_link_libraries(WaitSetImplTests ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES})
        add_gtest(WaitSetImplTests SOURCES ${WAITSET_IMPL_TESTS_SOURCE})
    endif()
endif()

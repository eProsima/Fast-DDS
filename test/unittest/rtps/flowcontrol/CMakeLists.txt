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
    if(GTest_FOUND)
        if(WIN32)
            add_definitions(-D_WIN32_WINNT=0x0601)
        endif()

        set(FLOWCONTROLLER_COMMON_SOURCE
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/LocatorSelectorSender.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/Log.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/OStreamConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutConsumer.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/ThroughputControllerDescriptor.cpp
            )

        set(FLOWCONTROLLERFACTORYTESTS_SOURCE
            ${FLOWCONTROLLER_COMMON_SOURCE}
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerFactory.cpp
            FlowControllerFactoryTests.cpp
            )

        add_executable(FlowControllerFactoryTests ${FLOWCONTROLLERFACTORYTESTS_SOURCE})
        target_compile_definitions(FlowControllerFactoryTests PRIVATE FASTRTPS_NO_LIB
            $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
            $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
            )
        target_include_directories(FlowControllerFactoryTests PRIVATE
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSWriter
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSMessageGroup
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/cpp
            )
        target_link_libraries(ThroughputControllerTests GTest::gtest GTest::gmock)
        if(MSVC OR MSVC_IDE)
            target_link_libraries(FlowControllerFactoryTests ${PRIVACY}
                iphlpapi Shlwapi
                )
        endif()
        add_gtest(FlowControllerFactoryTests SOURCES ${FLOWCONTROLLERFACTORYTESTS_SOURCE})

        set(FLOWCONTROLLERPUBLISHMODESTESTS_SOURCE
            ${FLOWCONTROLLER_COMMON_SOURCE}
            FlowControllerPublishModesTests.cpp
            )

        add_executable(FlowControllerPublishModesTests ${FLOWCONTROLLERPUBLISHMODESTESTS_SOURCE})
        target_compile_definitions(FlowControllerPublishModesTests PRIVATE FASTRTPS_NO_LIB
            $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
            $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
            )
        target_include_directories(FlowControllerPublishModesTests PRIVATE ${GTEST_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSWriter
            ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSMessageGroup
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/cpp
            )
        target_link_libraries(FlowControllerPublishModesTests ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES})
        if(MSVC OR MSVC_IDE)
            target_link_libraries(FlowControllerPublishModesTests ${PRIVACY}
                iphlpapi Shlwapi
                )
        endif()
        add_gtest(FlowControllerPublishModesTests SOURCES ${FLOWCONTROLLERPUBLISHMODESTESTS_SOURCE})
    endif()
endif()

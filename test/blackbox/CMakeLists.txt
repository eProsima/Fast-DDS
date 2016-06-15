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

if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER) AND fastcdr_FOUND)
    include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)
    check_gtest()
	
    if(GTEST_FOUND)
        if(WIN32)
            add_definitions(-D_WIN32_WINNT=0x0601)
        endif()


        ###############################################################################
        # Unit tests
        ###############################################################################
        set(BLACKBOXTESTS_SOURCE BlackboxTests.cpp
            types/HelloWorld.cpp
            types/HelloWorldType.cpp
            types/Data64kb.cpp
            types/Data64kbType.cpp
            types/Data1mb.cpp
            types/Data1mbType.cpp
            RTPSAsSocketReader.cpp
            RTPSAsSocketWriter.cpp
            RTPSWithRegistrationReader.cpp
            RTPSWithRegistrationWriter.cpp
            ReqRepHelloWorldRequester.cpp
            ReqRepHelloWorldReplier.cpp
            )
        add_executable(BlackboxTests ${BLACKBOXTESTS_SOURCE})
        add_gtest(BlackboxTests ${BLACKBOXTESTS_SOURCE})
        target_include_directories(BlackboxTests PRIVATE ${Boost_INCLUDE_DIR} ${GTEST_INCLUDE_DIRS})
        target_link_libraries(BlackboxTests fastrtps fastcdr ${GTEST_LIBRARIES})
    endif()
endif()


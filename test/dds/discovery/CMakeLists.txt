# Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima)
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
    find_package(Python3 COMPONENTS Interpreter)

    ###########################################################################
    # Binaries
    ###########################################################################
    set(DDS_PARTICIPANT_SOURCE
        ParticipantModule.cpp
        ParticipantMain.cpp
        )

    add_executable(DDSParticipantDiscovery ${DDS_PARTICIPANT_SOURCE})
    target_compile_definitions(DDSParticipantDiscovery PRIVATE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG>
        )
    target_link_libraries(DDSParticipantDiscovery
        fastcdr
        fastdds
        ${CMAKE_DL_LIBS}
        )

    ###########################################################################
    # Necessary files
    ###########################################################################
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/client_server_dynamic_discovery.py
        ${CMAKE_CURRENT_BINARY_DIR}/client_server_dynamic_discovery.py COPYONLY)

    string(RANDOM LENGTH 4 ALPHABET 0123456789 W_UNICAST_PORT_RANDOM_NUMBER)
    math(EXPR W_UNICAST_PORT_RANDOM_NUMBER "${W_UNICAST_PORT_RANDOM_NUMBER} + 0")
    if(W_UNICAST_PORT_RANDOM_NUMBER LESS 1025)
        math(EXPR W_UNICAST_PORT_RANDOM_NUMBER "${W_UNICAST_PORT_RANDOM_NUMBER} + 1024")
    endif()

    if(Python3_Interpreter_FOUND AND NOT APPLE AND NOT WIN32)
        add_test(NAME AddDiscoveryServerToListFromEnvironmentFile
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/client_server_dynamic_discovery.py)

        set_property(TEST AddDiscoveryServerToListFromEnvironmentFile PROPERTY ENVIRONMENT
            "CLIENT_SERVER_DYNAMIC_DISCOVERY_BIN=$<TARGET_FILE:DDSParticipantDiscovery>")
        set_property(TEST AddDiscoveryServerToListFromEnvironmentFile APPEND PROPERTY ENVIRONMENT
            "W_UNICAST_PORT_RANDOM_NUMBER=${W_UNICAST_PORT_RANDOM_NUMBER}")
        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST AddDiscoveryServerToListFromEnvironmentFile APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()
    endif()
endif()

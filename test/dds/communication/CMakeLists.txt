# Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
    find_package(PythonInterp 3)

    add_definitions(
        -DBOOST_ASIO_STANDALONE
        -DASIO_STANDALONE
        )

    include_directories(${Asio_INCLUDE_DIR})

    ###############################################################################
    # Binaries
    ###############################################################################
    set(DDS_PUBLISHER_SOURCE
        Publisher.cpp
        )
    add_executable(DDSSimpleCommunicationPublisher ${DDS_PUBLISHER_SOURCE})
    target_compile_definitions(DDSSimpleCommunicationPublisher PRIVATE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
    target_link_libraries(DDSSimpleCommunicationPublisher fastrtps fastcdr foonathan_memory ${CMAKE_DL_LIBS})

    set(DDS_SUBSCRIBER_SOURCE
        Subscriber.cpp
        )
    add_executable(DDSSimpleCommunicationSubscriber ${DDS_SUBSCRIBER_SOURCE})
    target_compile_definitions(DDSSimpleCommunicationSubscriber PRIVATE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
    target_link_libraries(DDSSimpleCommunicationSubscriber fastrtps fastcdr foonathan_memory ${CMAKE_DL_LIBS})

    ###############################################################################
    # Necessary files
    ###############################################################################
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_communication.py
        ${CMAKE_CURRENT_BINARY_DIR}/simple_communication.py COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/example_type.xml
        ${CMAKE_CURRENT_BINARY_DIR}/example_type.xml COPYONLY)
    if(SECURITY)
        configure_file(${PROJECT_SOURCE_DIR}/test/certs/maincacert.pem
            ${CMAKE_CURRENT_BINARY_DIR}/maincacert.pem COPYONLY)
        configure_file(${PROJECT_SOURCE_DIR}/test/certs/mainpubcert.pem
            ${CMAKE_CURRENT_BINARY_DIR}/mainpubcert.pem COPYONLY)
        configure_file(${PROJECT_SOURCE_DIR}/test/certs/mainpubkey.pem
            ${CMAKE_CURRENT_BINARY_DIR}/mainpubkey.pem COPYONLY)
        configure_file(${PROJECT_SOURCE_DIR}/test/certs/mainsubcert.pem
            ${CMAKE_CURRENT_BINARY_DIR}/mainsubcert.pem COPYONLY)
        configure_file(${PROJECT_SOURCE_DIR}/test/certs/mainsubkey.pem
            ${CMAKE_CURRENT_BINARY_DIR}/mainsubkey.pem COPYONLY)
        configure_file(${PROJECT_SOURCE_DIR}/test/certs/governance_helloworld_all_enable.smime
            ${CMAKE_CURRENT_BINARY_DIR}/governance_helloworld_all_enable.smime COPYONLY)
        configure_file(${PROJECT_SOURCE_DIR}/test/certs/permissions_helloworld.smime
            ${CMAKE_CURRENT_BINARY_DIR}/permissions_helloworld.smime COPYONLY)
    endif()

    if(PYTHONINTERP_FOUND)
        add_test(NAME DDSSimpleCommunicationTypeDiscovery
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/simple_communication.py)

        # Set test with label NoMemoryCheck
        set_property(TEST DDSSimpleCommunicationTypeDiscovery PROPERTY LABELS "NoMemoryCheck")

        set_property(TEST DDSSimpleCommunicationTypeDiscovery PROPERTY ENVIRONMENT
            "DDS_SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:DDSSimpleCommunicationPublisher>")
        set_property(TEST DDSSimpleCommunicationTypeDiscovery APPEND PROPERTY ENVIRONMENT
            "DDS_SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:DDSSimpleCommunicationSubscriber>")
        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST DDSSimpleCommunicationTypeDiscovery APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()
    endif()
endif()

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

    include_directories(${ASIO_INCLUDE_DIR})

    ###############################################################################
    # Binaries
    ###############################################################################

    # Dynamic types test
    set(DDS_PUBLISHER_DYNAMIC_SOURCE
        PublisherDynamic.cpp
        )
    add_executable(DDSSimpleCommunicationDynamicPublisher ${DDS_PUBLISHER_DYNAMIC_SOURCE})
    target_compile_definitions(DDSSimpleCommunicationDynamicPublisher PRIVATE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
    target_link_libraries(DDSSimpleCommunicationDynamicPublisher fastrtps fastcdr foonathan_memory ${CMAKE_DL_LIBS})

    set(DDS_SUBSCRIBER_DYNAMIC_SOURCE
        SubscriberDynamic.cpp
        )
    add_executable(DDSSimpleCommunicationDynamicSubscriber ${DDS_SUBSCRIBER_DYNAMIC_SOURCE})
    target_compile_definitions(DDSSimpleCommunicationDynamicSubscriber PRIVATE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
    target_link_libraries(DDSSimpleCommunicationDynamicSubscriber fastrtps fastcdr foonathan_memory ${CMAKE_DL_LIBS})

    # Standar tests
    set(COMMON_SOURCE ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorld.cpp
        ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorldType.cpp
        ${PROJECT_SOURCE_DIR}/test/blackbox/types/FixedSized.cpp
        ${PROJECT_SOURCE_DIR}/test/blackbox/types/FixedSizedType.cpp
        )

    set(DDS_PUBLISHER_SOURCE ${COMMON_SOURCE}
        PublisherModule.cpp
        PublisherMain.cpp
        )
    add_executable(DDSCommunicationPublisher ${DDS_PUBLISHER_SOURCE})
    target_compile_definitions(DDSCommunicationPublisher PRIVATE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
    target_include_directories(DDSCommunicationPublisher PRIVATE ${PROJECT_SOURCE_DIR}/test/blackbox)
    target_link_libraries(DDSCommunicationPublisher fastrtps fastcdr foonathan_memory ${CMAKE_DL_LIBS})

    set(DDS_SUBSCRIBER_SOURCE ${COMMON_SOURCE}
        SubscriberModule.cpp
        SubscriberMain.cpp
        )
    add_executable(DDSCommunicationSubscriber ${DDS_SUBSCRIBER_SOURCE})
    target_compile_definitions(DDSCommunicationSubscriber PRIVATE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
    target_include_directories(DDSCommunicationSubscriber PRIVATE ${PROJECT_SOURCE_DIR}/test/blackbox)
    target_link_libraries(DDSCommunicationSubscriber fastrtps fastcdr foonathan_memory ${CMAKE_DL_LIBS})

    set(DDS_PUBSUB_SOURCE ${COMMON_SOURCE}
        PublisherModule.cpp
        SubscriberModule.cpp
        PubSubMain.cpp
        )
    add_executable(DDSCommunicationPubSub ${DDS_PUBSUB_SOURCE})
    target_compile_definitions(DDSCommunicationPubSub PRIVATE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
    target_include_directories(DDSCommunicationPubSub PRIVATE ${PROJECT_SOURCE_DIR}/test/blackbox)
    target_link_libraries(DDSCommunicationPubSub fastrtps fastcdr foonathan_memory ${CMAKE_DL_LIBS})

    ###############################################################################
    # Necessary files
    ###############################################################################
    # Dynamic types test
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_communication_dynamic.py
        ${CMAKE_CURRENT_BINARY_DIR}/simple_communication_dynamic.py COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/example_type.xml
        ${CMAKE_CURRENT_BINARY_DIR}/example_type.xml COPYONLY)

    # Standard tests

    list(APPEND TEST_DEFINITIONS
            simple_communication
            besteffort_communication
            mix_communication
            simple_communication_fixed_type
            zero_copy_communication
            zero_copy_pub_communication
            zero_copy_sub_communication
            mix_zero_copy_communication
        )


    list(APPEND XML_CONFIGURATION_FILES
        simple_reliable.xml
        simple_besteffort.xml
        simple_reliable_zerocopy.xml
        simple_besteffort_zerocopy.xml
)
    # Python file
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_build.py
        ${CMAKE_CURRENT_BINARY_DIR}/test_build.py COPYONLY)

    # XML files
    foreach(XML_FILE IN LISTS XML_CONFIGURATION_FILES)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${XML_FILE}
            ${CMAKE_CURRENT_BINARY_DIR}/${XML_FILE} COPYONLY)
    endforeach()

    # JSON files
    foreach(TEST_FILE IN LISTS TEST_DEFINITIONS)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${TEST_FILE}.json
            ${CMAKE_CURRENT_BINARY_DIR}/${TEST_FILE}.json COPYONLY)
    endforeach()


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


    ###############################################################################
    # Tests specification
    ###############################################################################
    if(PYTHONINTERP_FOUND)
        # Dynamic types test
        add_test(NAME DDSSimpleCommunicationTypeDiscovery
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/simple_communication_dynamic.py)

        # Set test with label NoMemoryCheck
        set_property(TEST DDSSimpleCommunicationTypeDiscovery PROPERTY LABELS "NoMemoryCheck")

        set_property(TEST DDSSimpleCommunicationTypeDiscovery PROPERTY ENVIRONMENT
            "DDS_SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:DDSSimpleCommunicationDynamicPublisher>")
        set_property(TEST DDSSimpleCommunicationTypeDiscovery APPEND PROPERTY ENVIRONMENT
            "DDS_SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:DDSSimpleCommunicationDynamicSubscriber>")
        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST DDSSimpleCommunicationTypeDiscovery APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()

        # Standard types test
        set(TEST_BUILDER ${BINARY_TEST_DIR}test_build.py)

        foreach(TEST_DEFINITION IN LISTS TEST_DEFINITIONS)
            set(TEST_NAME DDSCommunication_${TEST_DEFINITION})
            add_test(
                NAME ${TEST_NAME}
                COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_BUILDER}
                ${TEST_DEFINITION}.json
            )

            # Set test with label NoMemoryCheck
            set_property(TEST ${TEST_NAME} PROPERTY LABELS "NoMemoryCheck")

            set_property(TEST ${TEST_NAME} PROPERTY ENVIRONMENT
                "DDS_SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:DDSCommunicationPublisher>")
            set_property(TEST ${TEST_NAME} APPEND PROPERTY ENVIRONMENT
                "DDS_SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:DDSCommunicationSubscriber>")
            set_property(TEST ${TEST_NAME} APPEND PROPERTY ENVIRONMENT
                "DDS_SIMPLE_COMMUNICATION_PUBSUB_BIN=$<TARGET_FILE:DDSCommunicationPubSub>")
            if(WIN32)
                string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
                set_property(TEST ${TEST_NAME} APPEND PROPERTY ENVIRONMENT
                    "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
            endif()
        endforeach()
    endif()
endif()

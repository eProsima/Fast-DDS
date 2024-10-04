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

find_package(Python3 COMPONENTS Interpreter)

add_definitions(
    -DBOOST_ASIO_STANDALONE
    -DASIO_STANDALONE
    )

include_directories(${Asio_INCLUDE_DIR})

###############################################################################
# Binaries
###############################################################################

# TODO Restore when Dynamic types are registered in TypeObjectRegistry
# # Dynamic types test
# set(DDS_PUBLISHER_DYNAMIC_SOURCE
#     PublisherDynamic.cpp
#     )
# add_executable(DDSSimpleCommunicationDynamicPublisher ${DDS_PUBLISHER_DYNAMIC_SOURCE})
# target_compile_definitions(DDSSimpleCommunicationDynamicPublisher PRIVATE
#     $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
#     $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
#     )
# target_link_libraries(DDSSimpleCommunicationDynamicPublisher fastdds fastcdr foonathan_memory ${CMAKE_DL_LIBS})

# set(DDS_SUBSCRIBER_DYNAMIC_SOURCE
#     SubscriberDynamic.cpp
#     )
# add_executable(DDSSimpleCommunicationDynamicSubscriber ${DDS_SUBSCRIBER_DYNAMIC_SOURCE})
# target_compile_definitions(DDSSimpleCommunicationDynamicSubscriber PRIVATE
#     $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
#     $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
#     )
# target_link_libraries(DDSSimpleCommunicationDynamicSubscriber fastdds fastcdr foonathan_memory ${CMAKE_DL_LIBS})

# Standard tests
set(COMMON_SOURCE
    ${PROJECT_SOURCE_DIR}/test/blackbox/types/FixedSizedPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/blackbox/types/FixedSizedTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorldPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorldTypeObjectSupport.cxx
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
target_link_libraries(DDSCommunicationPublisher fastdds fastcdr foonathan_memory ${CMAKE_DL_LIBS})

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
target_link_libraries(DDSCommunicationSubscriber fastdds fastcdr foonathan_memory ${CMAKE_DL_LIBS})

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
target_link_libraries(DDSCommunicationPubSub fastdds fastcdr foonathan_memory ${CMAKE_DL_LIBS})

###############################################################################
# Necessary files
###############################################################################
# TODO Restore when Dynamic types are registered in TypeObjectRegistry
# # Dynamic types test
# configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_communication_dynamic.py
#     ${CMAKE_CURRENT_BINARY_DIR}/simple_communication_dynamic.py COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/example_type_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/example_type_profile.xml COPYONLY)

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
    close_TCP_client
    simple_data_sharing_stress
    )


list(APPEND XML_CONFIGURATION_FILES
    simple_reliable_profile.xml
    simple_besteffort_profile.xml
    simple_reliable_zerocopy_profile.xml
    simple_besteffort_zerocopy_profile.xml
    TCP_server_profile.xml
    TCP_client_profile.xml
    liveliness_assertion_profile.xml
    liveliness_assertion.360_profile.xml
    shm_communication_subscriber_dies_while_processing_message_profile.xml
    )

list(APPEND PYTHON_FILES
    test_build.py
    liveliness_assertion.py
    automatic_liveliness_assertion.py
    shm_communication_subscriber_dies_while_processing_message.py
    two_publishers_communication.py
    )

# Python files
foreach(PY_FILE IN LISTS PYTHON_FILES)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${PY_FILE}
        ${CMAKE_CURRENT_BINARY_DIR}/${PY_FILE} COPYONLY)
endforeach()

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

# Cmake file
configure_file(${PROJECT_SOURCE_DIR}/cmake/common/test_wrapper.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/test_wrapper.cmake @ONLY)

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
if(Python3_Interpreter_FOUND)

    # TODO Restore when Dynamic types are registered in TypeObjectRegistry
    # # Dynamic types test
    # add_test(NAME DDSSimpleCommunicationTypeDiscovery
    #     COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/simple_communication_dynamic.py)

    # # Set test with label NoMemoryCheck
    # set_property(TEST DDSSimpleCommunicationTypeDiscovery PROPERTY LABELS "NoMemoryCheck")

    # set_property(TEST DDSSimpleCommunicationTypeDiscovery PROPERTY ENVIRONMENT
    #     "DDS_SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:DDSSimpleCommunicationDynamicPublisher>")
    # set_property(TEST DDSSimpleCommunicationTypeDiscovery APPEND PROPERTY ENVIRONMENT
    #     "DDS_SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:DDSSimpleCommunicationDynamicSubscriber>")
    # if(WIN32)
    #     string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
    #     set_property(TEST DDSSimpleCommunicationTypeDiscovery APPEND PROPERTY ENVIRONMENT
    #         "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
    # endif()

    # Standard types test
    set(TEST_BUILDER ${BINARY_TEST_DIR}test_build.py)

    foreach(TEST_DEFINITION IN LISTS TEST_DEFINITIONS)
        set(TEST_NAME DDSCommunication_${TEST_DEFINITION})
        add_test(
            NAME ${TEST_NAME}
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_BUILDER}
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

    # Liveliness tests
    add_test(NAME LivelinessAssertion
        COMMAND ${CMAKE_COMMAND} -DACTUAL_TEST=${PYTHON_EXECUTABLE} -DACTUAL_ARGS=${CMAKE_CURRENT_BINARY_DIR}/liveliness_assertion.py -P ${CMAKE_CURRENT_BINARY_DIR}/test_wrapper.cmake)
    # Set test with label NoMemoryCheck
    set_property(TEST LivelinessAssertion PROPERTY LABELS "NoMemoryCheck")

    set_property(TEST LivelinessAssertion PROPERTY ENVIRONMENT
        "DDS_SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:DDSCommunicationPublisher>")
    set_property(TEST LivelinessAssertion APPEND PROPERTY ENVIRONMENT
        "DDS_SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:DDSCommunicationSubscriber>")
    if(WIN32)
        string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
        set_property(TEST LivelinessAssertion APPEND PROPERTY ENVIRONMENT
            "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
    endif()

    add_test(NAME AutomaticLivelinessAssertion
        COMMAND ${CMAKE_COMMAND} -DACTUAL_TEST=${PYTHON_EXECUTABLE} -DACTUAL_ARGS=${CMAKE_CURRENT_BINARY_DIR}/automatic_liveliness_assertion.py -P ${CMAKE_CURRENT_BINARY_DIR}/test_wrapper.cmake)
    # Set test with label NoMemoryCheck
    set_property(TEST AutomaticLivelinessAssertion PROPERTY LABELS "NoMemoryCheck")

    set_property(TEST AutomaticLivelinessAssertion PROPERTY ENVIRONMENT
        "DDS_SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:DDSCommunicationPublisher>")
    set_property(TEST AutomaticLivelinessAssertion APPEND PROPERTY ENVIRONMENT
        "DDS_SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:DDSCommunicationSubscriber>")
    if(WIN32)
        string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
        set_property(TEST AutomaticLivelinessAssertion APPEND PROPERTY ENVIRONMENT
            "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
    endif()

    # Shm test
    add_test(NAME SHMCommunicationSubscriberDiesWhileProcessingMessage
    COMMAND ${CMAKE_COMMAND} -DACTUAL_TEST=${PYTHON_EXECUTABLE} -DACTUAL_ARGS=${CMAKE_CURRENT_BINARY_DIR}/shm_communication_subscriber_dies_while_processing_message.py -P ${CMAKE_CURRENT_BINARY_DIR}/test_wrapper.cmake)

    # Set test with label NoMemoryCheck
    set_property(TEST SHMCommunicationSubscriberDiesWhileProcessingMessage PROPERTY LABELS "NoMemoryCheck")

    set_property(TEST SHMCommunicationSubscriberDiesWhileProcessingMessage PROPERTY ENVIRONMENT
        "DDS_SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:DDSCommunicationPublisher>")
    set_property(TEST SHMCommunicationSubscriberDiesWhileProcessingMessage APPEND PROPERTY ENVIRONMENT
        "DDS_SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:DDSCommunicationSubscriber>")
    set_property(TEST SHMCommunicationSubscriberDiesWhileProcessingMessage APPEND PROPERTY ENVIRONMENT
        "XML_FILE=shm_communication_subscriber_dies_while_processing_message_profile.xml")
    if(WIN32)
        string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
        set_property(TEST SHMCommunicationSubscriberDiesWhileProcessingMessage APPEND PROPERTY ENVIRONMENT
            "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
    endif()

    add_test(NAME TwoPublishersCommunicationBestEffort
    COMMAND ${CMAKE_COMMAND} -DACTUAL_TEST=${PYTHON_EXECUTABLE} -DACTUAL_ARGS=${CMAKE_CURRENT_BINARY_DIR}/two_publishers_communication.py -P ${CMAKE_CURRENT_BINARY_DIR}/test_wrapper.cmake)

    # Set test with label NoMemoryCheck
    set_property(TEST TwoPublishersCommunicationBestEffort PROPERTY LABELS "NoMemoryCheck")

    set_property(TEST TwoPublishersCommunicationBestEffort PROPERTY ENVIRONMENT
        "DDS_SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:DDSCommunicationPublisher>")
    set_property(TEST TwoPublishersCommunicationBestEffort APPEND PROPERTY ENVIRONMENT
        "DDS_SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:DDSCommunicationSubscriber>")
    set_property(TEST TwoPublishersCommunicationBestEffort APPEND PROPERTY ENVIRONMENT
        "XML_FILE=simple_besteffort_profile.xml")
    if(WIN32)
        string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
        set_property(TEST TwoPublishersCommunicationBestEffort APPEND PROPERTY ENVIRONMENT
            "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
    endif()

    add_test(NAME TwoPublishersCommunicationReliable
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/two_publishers_communication.py)

    # Set test with label NoMemoryCheck
    set_property(TEST TwoPublishersCommunicationReliable PROPERTY LABELS "NoMemoryCheck")

    set_property(TEST TwoPublishersCommunicationReliable PROPERTY ENVIRONMENT
        "DDS_SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:DDSCommunicationPublisher>")
    set_property(TEST TwoPublishersCommunicationReliable APPEND PROPERTY ENVIRONMENT
        "DDS_SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:DDSCommunicationSubscriber>")
    set_property(TEST TwoPublishersCommunicationReliable APPEND PROPERTY ENVIRONMENT
        "XML_FILE=simple_reliable_profile.xml")
    if(WIN32)
        string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
        set_property(TEST TwoPublishersCommunicationReliable APPEND PROPERTY ENVIRONMENT
            "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
    endif()

endif()

if(UNIX AND NOT(APPLE) AND NOT(QNXNTO) AND NOT(ANDROID))
    add_subdirectory(dyn_network)
endif()

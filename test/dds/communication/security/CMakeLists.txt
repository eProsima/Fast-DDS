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

find_package(PythonInterp 3)

add_definitions(
    -DBOOST_ASIO_STANDALONE
    -DASIO_STANDALONE
    )

include_directories(${Asio_INCLUDE_DIR})

###############################################################################
# Binaries
###############################################################################
set(COMMON_SOURCE
    ${FASTDDS_BASIC_DIR}/test/blackbox/types/HelloWorldPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/blackbox/types/HelloWorldTypeObjectSupport.cxx
    ${FASTDDS_BASIC_DIR}/test/blackbox/types/FixedSizedPubSubTypes.cxx
    ${FASTDDS_BASIC_DIR}/test/blackbox/types/FixedSizedTypeObjectSupport.cxx
    )

set(DDS_PUBLISHER_SOURCE ${COMMON_SOURCE}
    PublisherModule.cpp
    PublisherMain.cpp
    )
add_executable(CommunicationPublisher ${DDS_PUBLISHER_SOURCE})
target_compile_definitions(CommunicationPublisher PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(CommunicationPublisher PRIVATE ${FASTDDS_BASIC_DIR}/test/blackbox)
target_link_libraries(CommunicationPublisher fastdds fastcdr foonathan_memory ${CMAKE_DL_LIBS})

set(DDS_SUBSCRIBER_SOURCE ${COMMON_SOURCE}
    SubscriberModule.cpp
    SubscriberMain.cpp
    )
add_executable(CommunicationSubscriber ${DDS_SUBSCRIBER_SOURCE})
target_compile_definitions(CommunicationSubscriber PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(CommunicationSubscriber PRIVATE ${FASTDDS_BASIC_DIR}/test/blackbox)
target_link_libraries(CommunicationSubscriber fastdds fastcdr foonathan_memory ${CMAKE_DL_LIBS})

###############################################################################
# Necessary files
###############################################################################

list(APPEND XML_CONFIGURATION_FILES
        simple_secure_besteffort_pub_profile.xml
        simple_secure_besteffort_sub_profile.xml
        secure_msg_crypto_besteffort_pub_profile.xml
        secure_msg_crypto_besteffort_sub_profile.xml
        secure_msg_submsg_crypto_besteffort_pub_profile.xml
        secure_msg_submsg_crypto_besteffort_sub_profile.xml
        secure_submsg_crypto_besteffort_pub_profile.xml
        secure_submsg_crypto_besteffort_sub_profile.xml
        simple_secure_allow_unauthenticated_pub_profile.xml
        simple_allow_unauthenticated_sub_profile.xml
    )

list(APPEND PYTHON_FILES
    simple_communication
    multiple_subs_secure_crypto_communication
    )

# XML files
foreach(XML_FILE IN LISTS XML_CONFIGURATION_FILES)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${XML_FILE}
        ${CMAKE_CURRENT_BINARY_DIR}/${XML_FILE} COPYONLY)
endforeach()

# Python files
foreach(PY_FILE IN LISTS PYTHON_FILES)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${PY_FILE}.py
    ${CMAKE_CURRENT_BINARY_DIR}/${PY_FILE}.py COPYONLY)
endforeach()

# Cmake file
configure_file(${FASTDDS_BASIC_DIR}/cmake/common/test_wrapper.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/test_wrapper.cmake @ONLY)

if(SECURITY)
    configure_file(${FASTDDS_BASIC_DIR}/test/certs/maincacert.pem
        ${CMAKE_CURRENT_BINARY_DIR}/maincacert.pem COPYONLY)
    configure_file(${FASTDDS_BASIC_DIR}/test/certs/mainpubcert.pem
        ${CMAKE_CURRENT_BINARY_DIR}/mainpubcert.pem COPYONLY)
    configure_file(${FASTDDS_BASIC_DIR}/test/certs/mainpubkey.pem
        ${CMAKE_CURRENT_BINARY_DIR}/mainpubkey.pem COPYONLY)
    configure_file(${FASTDDS_BASIC_DIR}/test/certs/mainsubcert.pem
        ${CMAKE_CURRENT_BINARY_DIR}/mainsubcert.pem COPYONLY)
    configure_file(${FASTDDS_BASIC_DIR}/test/certs/mainsubkey.pem
        ${CMAKE_CURRENT_BINARY_DIR}/mainsubkey.pem COPYONLY)
    configure_file(${FASTDDS_BASIC_DIR}/test/certs/governance_helloworld_all_enable.smime
        ${CMAKE_CURRENT_BINARY_DIR}/governance_helloworld_all_enable.smime COPYONLY)
    configure_file(${FASTDDS_BASIC_DIR}/test/certs/governance_disable_discovery_disable_rtps_helloworld_all_enable.smime
        ${CMAKE_CURRENT_BINARY_DIR}/governance_disable_discovery_disable_rtps_helloworld_all_enable.smime COPYONLY)
    configure_file(${FASTDDS_BASIC_DIR}/test/certs/governance_disable_discovery_helloworld_all_enable.smime
        ${CMAKE_CURRENT_BINARY_DIR}/governance_disable_discovery_helloworld_all_enable.smime COPYONLY)
    configure_file(${FASTDDS_BASIC_DIR}/test/certs/governance_disable_rtps_helloworld_all_enable.smime
        ${CMAKE_CURRENT_BINARY_DIR}/governance_disable_rtps_helloworld_all_enable.smime COPYONLY)
    configure_file(${FASTDDS_BASIC_DIR}/test/certs/permissions_helloworld.smime
        ${CMAKE_CURRENT_BINARY_DIR}/permissions_helloworld.smime COPYONLY)
    configure_file(${FASTDDS_BASIC_DIR}/test/certs/governance_allow_unauth_all_disabled_access_none.smime
        ${CMAKE_CURRENT_BINARY_DIR}/governance_allow_unauth_all_disabled_access_none.smime COPYONLY)
    configure_file(${FASTDDS_BASIC_DIR}/test/certs/permissions_helloworld_securehelloworld.smime
        ${CMAKE_CURRENT_BINARY_DIR}/permissions_helloworld_securehelloworld.smime COPYONLY)

    if(PYTHONINTERP_FOUND)

        add_test(NAME SimpleCommunicationSecureBestEffort
            COMMAND ${CMAKE_COMMAND} -DACTUAL_TEST=${PYTHON_EXECUTABLE} -DACTUAL_ARGS=${CMAKE_CURRENT_BINARY_DIR}/simple_communication.py -P ${CMAKE_CURRENT_BINARY_DIR}/test_wrapper.cmake)

        # Set test with label NoMemoryCheck
        set_property(TEST SimpleCommunicationSecureBestEffort PROPERTY LABELS "NoMemoryCheck")

        set_property(TEST SimpleCommunicationSecureBestEffort PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:CommunicationPublisher>")
        set_property(TEST SimpleCommunicationSecureBestEffort APPEND PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:CommunicationSubscriber>")
        set_property(TEST SimpleCommunicationSecureBestEffort APPEND PROPERTY ENVIRONMENT
            "XML_FILE_PUB=simple_secure_besteffort_pub_profile.xml")
        set_property(TEST SimpleCommunicationSecureBestEffort APPEND PROPERTY ENVIRONMENT
            "XML_FILE_SUB=simple_secure_besteffort_sub_profile.xml")
        set_property(TEST SimpleCommunicationSecureBestEffort APPEND PROPERTY ENVIRONMENT
            "EXTRA_PUB_ARG=--wait 2")
        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST SimpleCommunicationSecureBestEffort APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()

        add_test(NAME SimpleCommunicationSecureMsgCryptoBestEffort
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/multiple_subs_secure_crypto_communication.py
            --pub $<TARGET_FILE:CommunicationPublisher>
            --xml-pub secure_msg_crypto_besteffort_pub_profile.xml
            --sub $<TARGET_FILE:CommunicationSubscriber>
            --xml-sub secure_msg_crypto_besteffort_sub_profile.xml
            --samples 10 --wait 2 --n-subs 5)

        # Set test with label NoMemoryCheck
        set_property(TEST SimpleCommunicationSecureMsgCryptoBestEffort PROPERTY LABELS "NoMemoryCheck")

        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST SimpleCommunicationSecureMsgCryptoBestEffort APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()

        add_test(NAME SimpleCommunicationSecureMsgSubmsgCryptoBestEffort
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/multiple_subs_secure_crypto_communication.py
            --pub $<TARGET_FILE:CommunicationPublisher>
            --xml-pub secure_msg_submsg_crypto_besteffort_pub_profile.xml
            --sub $<TARGET_FILE:CommunicationSubscriber>
            --xml-sub secure_msg_submsg_crypto_besteffort_sub_profile.xml
            --samples 10 --wait 2 --n-subs 5)

        # Set test with label NoMemoryCheck
        set_property(TEST SimpleCommunicationSecureMsgSubmsgCryptoBestEffort PROPERTY LABELS "NoMemoryCheck")

        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST SimpleCommunicationSecureMsgSubmsgCryptoBestEffort APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()

        add_test(NAME SimpleCommunicationSecureSubmsgCryptoBestEffort
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/multiple_subs_secure_crypto_communication.py
            --pub $<TARGET_FILE:CommunicationPublisher>
            --xml-pub secure_submsg_crypto_besteffort_pub_profile.xml
            --sub $<TARGET_FILE:CommunicationSubscriber>
            --xml-sub secure_submsg_crypto_besteffort_sub_profile.xml
            --samples 10 --wait 2 --n-subs 5)

        # Set test with label NoMemoryCheck
        set_property(TEST SimpleCommunicationSecureSubmsgCryptoBestEffort PROPERTY LABELS "NoMemoryCheck")

        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST SimpleCommunicationSecureSubmsgCryptoBestEffort APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()

        add_test(NAME AllowUnauthenticatedSimplePubSecureNoRTPSProtectionSubNonSecure
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/multiple_subs_secure_crypto_communication.py
            --n-subs 1
            --pub $<TARGET_FILE:CommunicationPublisher>
            --xml-pub simple_secure_allow_unauthenticated_pub_profile.xml
            --sub $<TARGET_FILE:CommunicationSubscriber>
            --xml-sub simple_allow_unauthenticated_sub_profile.xml
            --samples 10)

        # Set test with label NoMemoryCheck
        set_property(TEST AllowUnauthenticatedSimplePubSecureNoRTPSProtectionSubNonSecure PROPERTY LABELS "NoMemoryCheck")

        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST AllowUnauthenticatedSimplePubSecureNoRTPSProtectionSubNonSecure APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()

    endif()

endif()

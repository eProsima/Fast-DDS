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
    find_package(PythonInterp 3)

    add_definitions(
        -DBOOST_ASIO_STANDALONE
        -DASIO_STANDALONE
        )

    include_directories(${ASIO_INCLUDE_DIR})

    ###############################################################################
    # Binaries
    ###############################################################################
    set(COMMON_SOURCE ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorld.cpp
        ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorldType.cpp
        ${PROJECT_SOURCE_DIR}/test/blackbox/types/FixedSized.cpp
        ${PROJECT_SOURCE_DIR}/test/blackbox/types/FixedSizedType.cpp
        )

    set(PUBLISHER_SOURCE ${COMMON_SOURCE}
        Publisher.cpp
        PublisherMain.cpp
        )
    add_executable(SimpleCommunicationPublisher ${PUBLISHER_SOURCE})
    target_compile_definitions(SimpleCommunicationPublisher PRIVATE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
    target_include_directories(SimpleCommunicationPublisher PRIVATE ${PROJECT_SOURCE_DIR}/test/blackbox)
    target_link_libraries(SimpleCommunicationPublisher fastrtps fastcdr foonathan_memory ${CMAKE_DL_LIBS})

    set(SUBSCRIBER_SOURCE ${COMMON_SOURCE}
        Subscriber.cpp
        SubscriberMain.cpp
        )
    add_executable(SimpleCommunicationSubscriber ${SUBSCRIBER_SOURCE})
    target_compile_definitions(SimpleCommunicationSubscriber PRIVATE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
    target_include_directories(SimpleCommunicationSubscriber PRIVATE ${PROJECT_SOURCE_DIR}/test/blackbox)
    target_link_libraries(SimpleCommunicationSubscriber fastrtps fastcdr foonathan_memory ${CMAKE_DL_LIBS})

    set(PUBSUB_SOURCE ${COMMON_SOURCE}
        Publisher.cpp
        Subscriber.cpp
        PubSubMain.cpp
        )

    add_executable(SimpleCommunicationPubSub ${PUBSUB_SOURCE})
    target_compile_definitions(SimpleCommunicationPubSub PRIVATE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
    target_include_directories(SimpleCommunicationPubSub PRIVATE ${PROJECT_SOURCE_DIR}/test/blackbox)
    target_link_libraries(SimpleCommunicationPubSub fastrtps fastcdr foonathan_memory ${CMAKE_DL_LIBS})

    ###############################################################################
    # Necessary files
    ###############################################################################
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_communication.py
        ${CMAKE_CURRENT_BINARY_DIR}/simple_communication.py COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_mix_communication.py
        ${CMAKE_CURRENT_BINARY_DIR}/simple_mix_communication.py COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/two_publishers_communication.py
        ${CMAKE_CURRENT_BINARY_DIR}/two_publishers_communication.py COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/liveliness_assertion.py
        ${CMAKE_CURRENT_BINARY_DIR}/liveliness_assertion.py COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/automatic_liveliness_assertion.py
        ${CMAKE_CURRENT_BINARY_DIR}/automatic_liveliness_assertion.py COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_besteffort.xml
        ${CMAKE_CURRENT_BINARY_DIR}/simple_besteffort.xml COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_reliable.xml
        ${CMAKE_CURRENT_BINARY_DIR}/simple_reliable.xml COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_reliable_besteffort.xml
        ${CMAKE_CURRENT_BINARY_DIR}/simple_reliable_besteffort.xml COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_secure_besteffort_pub.xml
        ${CMAKE_CURRENT_BINARY_DIR}/simple_secure_besteffort_pub.xml COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_secure_besteffort_sub.xml
        ${CMAKE_CURRENT_BINARY_DIR}/simple_secure_besteffort_sub.xml COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/liveliness_assertion.xml
        ${CMAKE_CURRENT_BINARY_DIR}/liveliness_assertion.xml COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/liveliness_assertion.120.xml
        ${CMAKE_CURRENT_BINARY_DIR}/liveliness_assertion.120.xml COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/liveliness_assertion.360.xml
        ${CMAKE_CURRENT_BINARY_DIR}/liveliness_assertion.360.xml COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/multiple_subs_secure_crypto_communication.py
        ${CMAKE_CURRENT_BINARY_DIR}/multiple_subs_secure_crypto_communication.py COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/secure_msg_crypto_besteffort_pub.xml
        ${CMAKE_CURRENT_BINARY_DIR}/secure_msg_crypto_besteffort_pub.xml COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/secure_msg_crypto_besteffort_sub.xml
        ${CMAKE_CURRENT_BINARY_DIR}/secure_msg_crypto_besteffort_sub.xml COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/secure_msg_submsg_crypto_besteffort_pub.xml
        ${CMAKE_CURRENT_BINARY_DIR}/secure_msg_submsg_crypto_besteffort_pub.xml COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/secure_msg_submsg_crypto_besteffort_sub.xml
        ${CMAKE_CURRENT_BINARY_DIR}/secure_msg_submsg_crypto_besteffort_sub.xml COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/secure_submsg_crypto_besteffort_pub.xml
        ${CMAKE_CURRENT_BINARY_DIR}/secure_submsg_crypto_besteffort_pub.xml COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/secure_submsg_crypto_besteffort_sub.xml
        ${CMAKE_CURRENT_BINARY_DIR}/secure_submsg_crypto_besteffort_sub.xml COPYONLY)
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
        add_test(NAME SimpleCommunicationBestEffort
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/simple_communication.py)

        # Set test with label NoMemoryCheck
        set_property(TEST SimpleCommunicationBestEffort PROPERTY LABELS "NoMemoryCheck")

        set_property(TEST SimpleCommunicationBestEffort PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:SimpleCommunicationPublisher>")
        set_property(TEST SimpleCommunicationBestEffort APPEND PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:SimpleCommunicationSubscriber>")
        set_property(TEST SimpleCommunicationBestEffort APPEND PROPERTY ENVIRONMENT
            "XML_FILE=simple_besteffort.xml")
        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST SimpleCommunicationBestEffort APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()

        add_test(NAME SimpleCommunicationReliable
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/simple_communication.py)

        # Set test with label NoMemoryCheck
        set_property(TEST SimpleCommunicationReliable PROPERTY LABELS "NoMemoryCheck")

        set_property(TEST SimpleCommunicationReliable PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:SimpleCommunicationPublisher>")
        set_property(TEST SimpleCommunicationReliable APPEND PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:SimpleCommunicationSubscriber>")
        set_property(TEST SimpleCommunicationReliable APPEND PROPERTY ENVIRONMENT
            "XML_FILE=simple_reliable.xml")
        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST SimpleCommunicationReliable APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()

        add_test(NAME SimpleCommunicationReliableBestEffort
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/simple_communication.py)

        # Set test with label NoMemoryCheck
        set_property(TEST SimpleCommunicationReliableBestEffort PROPERTY LABELS "NoMemoryCheck")

        set_property(TEST SimpleCommunicationReliableBestEffort PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:SimpleCommunicationPublisher>")
        set_property(TEST SimpleCommunicationReliableBestEffort APPEND PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:SimpleCommunicationSubscriber>")
        set_property(TEST SimpleCommunicationReliableBestEffort APPEND PROPERTY ENVIRONMENT
            "XML_FILE=simple_reliable_besteffort.xml")
        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST SimpleCommunicationReliableBestEffort APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()

        add_test(NAME SimpleMixCommunicationBestEffort
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/simple_mix_communication.py)

        # Set test with label NoMemoryCheck
        set_property(TEST SimpleMixCommunicationBestEffort PROPERTY LABELS "NoMemoryCheck")

        set_property(TEST SimpleMixCommunicationBestEffort PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:SimpleCommunicationPublisher>")
        set_property(TEST SimpleMixCommunicationBestEffort APPEND PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:SimpleCommunicationPubSub>")
        set_property(TEST SimpleMixCommunicationBestEffort APPEND PROPERTY ENVIRONMENT
            "XML_FILE=simple_besteffort.xml")
        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST SimpleMixCommunicationBestEffort APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()

        add_test(NAME SimpleMixCommunicationReliable
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/simple_mix_communication.py)

        # Set test with label NoMemoryCheck
        set_property(TEST SimpleMixCommunicationReliable PROPERTY LABELS "NoMemoryCheck")

        set_property(TEST SimpleMixCommunicationReliable PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:SimpleCommunicationPublisher>")
        set_property(TEST SimpleMixCommunicationReliable APPEND PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:SimpleCommunicationPubSub>")
        set_property(TEST SimpleMixCommunicationReliable APPEND PROPERTY ENVIRONMENT
            "XML_FILE=simple_reliable.xml")
        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST SimpleMixCommunicationReliable APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()

        if(SECURITY)
            add_test(NAME SimpleCommunicationSecureBestEffort
                COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/simple_communication.py)

            # Set test with label NoMemoryCheck
            set_property(TEST SimpleCommunicationSecureBestEffort PROPERTY LABELS "NoMemoryCheck")

            set_property(TEST SimpleCommunicationSecureBestEffort PROPERTY ENVIRONMENT
                "SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:SimpleCommunicationPublisher>")
            set_property(TEST SimpleCommunicationSecureBestEffort APPEND PROPERTY ENVIRONMENT
                "SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:SimpleCommunicationSubscriber>")
            set_property(TEST SimpleCommunicationSecureBestEffort APPEND PROPERTY ENVIRONMENT
                "XML_FILE_PUB=simple_secure_besteffort_pub.xml")
            set_property(TEST SimpleCommunicationSecureBestEffort APPEND PROPERTY ENVIRONMENT
                "XML_FILE_SUB=simple_secure_besteffort_sub.xml")
            set_property(TEST SimpleCommunicationSecureBestEffort APPEND PROPERTY ENVIRONMENT
                "EXTRA_PUB_ARG=--wait 2")
            if(WIN32)
                string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
                set_property(TEST SimpleCommunicationSecureBestEffort APPEND PROPERTY ENVIRONMENT
                    "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
            endif()

            add_test(NAME SimpleCommunicationSecureMsgCryptoBestEffort
                COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/multiple_subs_secure_crypto_communication.py
                --pub $<TARGET_FILE:SimpleCommunicationPublisher>
                --xml-pub secure_msg_crypto_besteffort_pub.xml
                --sub $<TARGET_FILE:SimpleCommunicationSubscriber>
                --xml-sub secure_msg_crypto_besteffort_sub.xml
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
                --pub $<TARGET_FILE:SimpleCommunicationPublisher>
                --xml-pub secure_msg_submsg_crypto_besteffort_pub.xml
                --sub $<TARGET_FILE:SimpleCommunicationSubscriber>
                --xml-sub secure_msg_submsg_crypto_besteffort_sub.xml
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
                --pub $<TARGET_FILE:SimpleCommunicationPublisher>
                --xml-pub secure_submsg_crypto_besteffort_pub.xml
                --sub $<TARGET_FILE:SimpleCommunicationSubscriber>
                --xml-sub secure_submsg_crypto_besteffort_sub.xml
                --samples 10 --wait 2 --n-subs 5)

            # Set test with label NoMemoryCheck
            set_property(TEST SimpleCommunicationSecureSubmsgCryptoBestEffort PROPERTY LABELS "NoMemoryCheck")

            if(WIN32)
                string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
                set_property(TEST SimpleCommunicationSecureSubmsgCryptoBestEffort APPEND PROPERTY ENVIRONMENT
                    "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
            endif()
        endif()

        add_test(NAME LivelinessAssertion
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/liveliness_assertion.py)

        # Set test with label NoMemoryCheck
        set_property(TEST LivelinessAssertion PROPERTY LABELS "NoMemoryCheck")

        set_property(TEST LivelinessAssertion PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:SimpleCommunicationPublisher>")
        set_property(TEST LivelinessAssertion APPEND PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:SimpleCommunicationSubscriber>")
        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST LivelinessAssertion APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()
        add_test(NAME AutomaticLivelinessAssertion
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/automatic_liveliness_assertion.py)

        # Set test with label NoMemoryCheck
        set_property(TEST AutomaticLivelinessAssertion PROPERTY LABELS "NoMemoryCheck")

        set_property(TEST AutomaticLivelinessAssertion PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:SimpleCommunicationPublisher>")
        set_property(TEST AutomaticLivelinessAssertion APPEND PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:SimpleCommunicationSubscriber>")
        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST AutomaticLivelinessAssertion APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()

        add_test(NAME TwoPublishersCommunicationBestEffort
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/two_publishers_communication.py)

        # Set test with label NoMemoryCheck
        set_property(TEST TwoPublishersCommunicationBestEffort PROPERTY LABELS "NoMemoryCheck")

        set_property(TEST TwoPublishersCommunicationBestEffort PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:SimpleCommunicationPublisher>")
        set_property(TEST TwoPublishersCommunicationBestEffort APPEND PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:SimpleCommunicationSubscriber>")
        set_property(TEST TwoPublishersCommunicationBestEffort APPEND PROPERTY ENVIRONMENT
            "XML_FILE=simple_besteffort.xml")
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
            "SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:SimpleCommunicationPublisher>")
        set_property(TEST TwoPublishersCommunicationReliable APPEND PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:SimpleCommunicationSubscriber>")
        set_property(TEST TwoPublishersCommunicationReliable APPEND PROPERTY ENVIRONMENT
            "XML_FILE=simple_reliable.xml")
        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST TwoPublishersCommunicationReliable APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()



        add_test(NAME SimpleCommunicationBestEffortFixed
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/simple_communication.py)

        # Set test with label NoMemoryCheck
        set_property(TEST SimpleCommunicationBestEffortFixed PROPERTY LABELS "NoMemoryCheck")

        set_property(TEST SimpleCommunicationBestEffortFixed PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:SimpleCommunicationPublisher>")
        set_property(TEST SimpleCommunicationBestEffortFixed APPEND PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:SimpleCommunicationSubscriber>")
        set_property(TEST SimpleCommunicationBestEffortFixed APPEND PROPERTY ENVIRONMENT
            "XML_FILE=simple_besteffort.xml")
        set_property(TEST SimpleCommunicationBestEffortFixed APPEND PROPERTY ENVIRONMENT
            "EXTRA_PUB_ARG=--fixed_type")
        set_property(TEST SimpleCommunicationBestEffortFixed APPEND PROPERTY ENVIRONMENT
            "EXTRA_SUB_ARG=--fixed_type")
        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST SimpleCommunicationBestEffortFixed APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()

    endif()
endif()

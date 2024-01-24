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

find_package(PythonInterp 3)

###############################################################################
# Binaries
###############################################################################
set(COMMON_SOURCE
    ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorld.cxx
    ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorldv1.cxx
    ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorldPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorldTypeObject.cxx
    ${PROJECT_SOURCE_DIR}/test/blackbox/types/FixedSized.cxx
    ${PROJECT_SOURCE_DIR}/test/blackbox/types/FixedSizedv1.cxx
    ${PROJECT_SOURCE_DIR}/test/blackbox/types/FixedSizedPubSubTypes.cxx
    )

set(PUBLISHER_SOURCE ${COMMON_SOURCE}
    Publisher.cpp
    PublisherMain.cpp
    )
add_executable(SimpleCommunicationPublisher ${PUBLISHER_SOURCE})
target_compile_definitions(SimpleCommunicationPublisher PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(SimpleCommunicationPublisher PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/blackbox)
target_link_libraries(SimpleCommunicationPublisher fastdds fastcdr foonathan_memory ${CMAKE_DL_LIBS})

set(SUBSCRIBER_SOURCE ${COMMON_SOURCE}
    Subscriber.cpp
    SubscriberMain.cpp
    )
add_executable(SimpleCommunicationSubscriber ${SUBSCRIBER_SOURCE})
target_compile_definitions(SimpleCommunicationSubscriber PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(SimpleCommunicationSubscriber PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/blackbox)
target_link_libraries(SimpleCommunicationSubscriber fastdds fastcdr foonathan_memory ${CMAKE_DL_LIBS})

set(PUBSUB_SOURCE ${COMMON_SOURCE}
    Publisher.cpp
    Subscriber.cpp
    PubSubMain.cpp
    )

add_executable(SimpleCommunicationPubSub ${PUBSUB_SOURCE})
target_compile_definitions(SimpleCommunicationPubSub PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(SimpleCommunicationPubSub PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/blackbox)
target_link_libraries(SimpleCommunicationPubSub fastdds fastcdr foonathan_memory ${CMAKE_DL_LIBS})

###############################################################################
# Necessary files
###############################################################################

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
    configure_file(${PROJECT_SOURCE_DIR}/test/certs/governance_disable_discovery_disable_rtps_helloworld_all_enable.smime
        ${CMAKE_CURRENT_BINARY_DIR}/governance_disable_discovery_disable_rtps_helloworld_all_enable.smime COPYONLY)
    configure_file(${PROJECT_SOURCE_DIR}/test/certs/governance_disable_discovery_helloworld_all_enable.smime
        ${CMAKE_CURRENT_BINARY_DIR}/governance_disable_discovery_helloworld_all_enable.smime COPYONLY)
    configure_file(${PROJECT_SOURCE_DIR}/test/certs/governance_disable_rtps_helloworld_all_enable.smime
        ${CMAKE_CURRENT_BINARY_DIR}/governance_disable_rtps_helloworld_all_enable.smime COPYONLY)
    configure_file(${PROJECT_SOURCE_DIR}/test/certs/permissions_helloworld.smime
        ${CMAKE_CURRENT_BINARY_DIR}/permissions_helloworld.smime COPYONLY)
    configure_file(${PROJECT_SOURCE_DIR}/test/certs/governance_allow_unauth_all_disabled_access_none.smime
        ${CMAKE_CURRENT_BINARY_DIR}/governance_allow_unauth_all_disabled_access_none.smime COPYONLY)
    configure_file(${PROJECT_SOURCE_DIR}/test/certs/permissions_helloworld_securehelloworld.smime
        ${CMAKE_CURRENT_BINARY_DIR}/permissions_helloworld_securehelloworld.smime COPYONLY)
endif()


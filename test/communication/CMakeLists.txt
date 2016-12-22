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
    find_package(PythonInterp 3 REQUIRED)

    ###############################################################################
    # Binaries
    ###############################################################################
    set(COMMON_SOURCE ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorld.cpp
        ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorldType.cpp
        )

    set(PUBLISHER_SOURCE ${COMMON_SOURCE}
        Publisher.cpp
        )
    add_executable(Publisher ${PUBLISHER_SOURCE})
    target_include_directories(Publisher PRIVATE ${PROJECT_SOURCE_DIR}/test/blackbox/types)
    target_link_libraries(Publisher fastrtps fastcdr)

    set(SUBSCRIBER_SOURCE ${COMMON_SOURCE}
        Subscriber.cpp
        )
    add_executable(Subscriber ${SUBSCRIBER_SOURCE})
    target_include_directories(Subscriber PRIVATE ${PROJECT_SOURCE_DIR}/test/blackbox/types)
    target_link_libraries(Subscriber fastrtps fastcdr)

    if(PYTHONINTERP_FOUND)
        add_test(NAME SimpleCommunication
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/simple_communication.py)

        # Set test with label NoMemoryCheck
        set_property(TEST SimpleCommunication PROPERTY LABELS "NoMemoryCheck")

        if(WIN32)
            set_property(TEST SimpleCommunication PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;${BOOST_LIBRARYDIR}\\;$ENV{PATH}")
        endif()
        set_property(TEST SimpleCommunication APPEND PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:Publisher>")
        set_property(TEST SimpleCommunication APPEND PROPERTY ENVIRONMENT
            "SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:Subscriber>")
    endif()
endif()

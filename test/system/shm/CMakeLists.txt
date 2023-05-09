# Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

###########################################################################
# Create and link executable                                              #
###########################################################################

file(GLOB SHM_RECOVERY_TEST_SOURCES_CXX "*.cxx")
file(GLOB SHM_RECOVERY_TEST_SOURCES_CPP "*.cpp")

add_executable(SHMRecoveryTest ${SHM_RECOVERY_TEST_SOURCES_CXX} ${SHM_RECOVERY_TEST_SOURCES_CPP})

target_compile_definitions(SHMRecoveryTest PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )

target_include_directories(SHMRecoveryTest PRIVATE ${Asio_INCLUDE_DIR})

target_link_libraries(
    SHMRecoveryTest
    fastrtps
    fastcdr
    foonathan_memory
    fastdds::optionparser
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
)

###########################################################################
# Create tests                                                            #
###########################################################################
find_package(PythonInterp 3 REQUIRED)
if(PYTHONINTERP_FOUND)

    # Add the test
    add_test(
        NAME shared_memory.recovery
        COMMAND ${PYTHON_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/shm_tests.py
    )

    # Add environment
    if(WIN32)
        set(WIN_PATH "$ENV{PATH}")
        get_target_property(LINK_LIBRARIES_ ${PROJECT_NAME} LINK_LIBRARIES)
        if(NOT "${LINK_LIBRARIES_}" STREQUAL "LINK_LIBRARIES_-NOTFOUND")
        list(APPEND LINK_LIBRARIES_ ${PROJECT_NAME})
            foreach(LIBRARY_LINKED ${LINK_LIBRARIES_})
                if(TARGET ${LIBRARY_LINKED})
                    # Check if is a real target or a target interface
                    get_target_property(dependency_type ${LIBRARY_LINKED} TYPE)
                    if(NOT dependency_type STREQUAL "INTERFACE_LIBRARY")
                        set(WIN_PATH "$<TARGET_FILE_DIR:${LIBRARY_LINKED}>;${WIN_PATH}")
                    endif()
                    unset(dependency_type)
                endif()
            endforeach()
        endif()
        string(REPLACE ";" "\\;" WIN_PATH "${WIN_PATH}")
    endif()

        # Set test properties
    set_property(
        TEST shared_memory.recovery
        PROPERTY LABELS "NoMemoryCheck"
    )
        
    if(WIN32)
        set_property(
            TEST shared_memory.recovery
            APPEND PROPERTY ENVIRONMENT "PATH=${WIN_PATH}")
    endif() 

    set_property(
        TEST shared_memory.recovery
        PROPERTY TIMEOUT 180
    )

endif()

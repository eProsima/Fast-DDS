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

if(PROFILING_TESTS)

    add_definitions(
      -DBOOST_ASIO_STANDALONE
      -DASIO_STANDALONE
    )

    include_directories(${ASIO_INCLUDE_DIR})

    set(RTPSCOMMSPROFILING_SOURCE
        RTPSCommsProfiling.cpp
        RTPSAsSocketReader.cpp
        RTPSAsSocketWriter.cpp)

    add_executable(RTPSCommsProfiling ${RTPSCOMMSPROFILING_SOURCE})
    target_compile_definitions(RTPSCommsProfiling PRIVATE FASTRTPS_NO_LIB)
    target_include_directories(RTPSCommsProfiling PRIVATE ../ ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
    target_link_libraries(RTPSCommsProfiling fastrtps fastcdr ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

    set(RTPSPUBSUBPROFILING_SOURCE
        RTPSPubSubProfiling.cpp
        types/HelloWorld.cpp
        types/HelloWorldType.cpp)

    add_executable(RTPSPubSubProfiling ${RTPSPUBSUBPROFILING_SOURCE})
    target_compile_definitions(RTPSPubSubProfiling PRIVATE FASTRTPS_NO_LIB)
    target_include_directories(RTPSPubSubProfiling PRIVATE ../ ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
    target_link_libraries(RTPSPubSubProfiling fastrtps fastcdr ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

    add_subdirectory(memory)
    add_subdirectory(cycles)

endif()

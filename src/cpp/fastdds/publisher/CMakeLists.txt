# Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#{{{ Unit with the DataWriterHistory, the history of the DDS writers.
# A unit test not linking core replaces it with the DataWriterHistory mock.

fastdds_add_unit(dds::writer-history
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/DataWriterHistory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/DataWriterHistory.hpp
    DEPENDS
    log
    rtps::history
    rtps::writer-history
    time
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

#{{{ Unit with the DataWriter, which forwards to its DataWriterImpl.
# A unit test not linking the DDS entities replaces it with the DataWriter mock.

fastdds_add_unit(dds::data-writer
    SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/publisher/DataWriter.hpp

    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/DataWriter.cpp
    DEPENDS
    dds::condition
    dds::entities
    log
    time
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

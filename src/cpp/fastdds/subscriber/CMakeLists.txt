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

#{{{ Unit with the DataReaderHistory, the history of the DDS readers, which keeps the samples by instance.

fastdds_add_unit(dds::reader-history
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/history/DataReaderCacheChange.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/history/DataReaderHistory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/history/DataReaderHistory.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/history/DataReaderHistoryCounters.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/history/DataReaderInstance.hpp
    DEPENDS
    log
    rtps::history
    rtps::reader-base
    rtps::reader-history
    time
    xtypes # On Windows, TypeSupport.hpp makes every object emit the exported inline DynamicPubSubType() and its thunks
    INCLUDE_DIRECTORIES
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    foonathan_memory
    )

#}}}

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

# The statistics module is only built with FASTDDS_STATISTICS.
if(FASTDDS_STATISTICS)

    #{{{ Unit with the type support of the statistics and monitor service types.
    # The type objects are registered on the registry of the DomainParticipantFactory.

    fastdds_add_unit(statistics::types
        SOURCES
        types/monitorservice_types.hpp
        types/monitorservice_typesCdrAux.hpp
        types/monitorservice_typesCdrAux.ipp
        types/monitorservice_typesPubSubTypes.cxx
        types/monitorservice_typesPubSubTypes.hpp
        types/monitorservice_typesTypeObjectSupport.cxx
        types/monitorservice_typesTypeObjectSupport.hpp
        types/types.hpp
        types/typesCdrAux.hpp
        types/typesCdrAux.ipp
        types/typesPubSubTypes.cxx
        types/typesPubSubTypes.hpp
        types/typesTypeObjectSupport.cxx
        types/typesTypeObjectSupport.hpp
        DEPENDS
        dds::participant-factory
        log
        utils::network
        xtypes
        LINK_LIBRARIES
        PUBLIC
        fastcdr
        )

    #}}}

    #{{{ Unit with the StatisticsBase, the statistics part of a participant, with the conversions between the RTPS and the
    # statistics types. A unit test not linking core replaces it with the StatisticsBase mock.

    fastdds_add_unit(statistics::base
        SOURCES
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/GuidUtils.hpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/StatisticsBase.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/StatisticsBase.hpp
        DEPENDS
        log
        time
        INCLUDE_DIRECTORIES
        ${Asio_INCLUDE_DIR}
        LINK_LIBRARIES
        PUBLIC
        fastcdr
        )

    #}}}

    #{{{ Unit with the MonitorService, which publishes the status of the local entities of a participant.
    # A unit test not linking core replaces it with the MonitorService mock.

    fastdds_add_unit(statistics::monitor-service
        SOURCES
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/monitor-service/Interfaces.hpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/monitor-service/MonitorService.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/monitor-service/MonitorService.hpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/monitor-service/MonitorServiceListener.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/monitor-service/MonitorServiceListener.hpp
        DEPENDS
        dds::writer-history
        log
        rtps::basic
        rtps::builtin-data
        rtps::domain
        rtps::history
        rtps::timed-event
        rtps::writer-history
        statistics::base
        statistics::types
        time
        INCLUDE_DIRECTORIES
        ${Asio_INCLUDE_DIR}
        ${THIRDPARTY_BOOST_INCLUDE_DIR}
        LINK_LIBRARIES
        PUBLIC
        fastcdr
        foonathan_memory
        $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto$<$<BOOL:${WIN32}>:$<SEMICOLON>crypt32.lib>>
        )

    #}}}

    #{{{ Unit with the DomainParticipantStatisticsListener, which forwards the statistics of a participant to the statistics
    # DataWriters.

    fastdds_add_unit(statistics::participant-listener
        SOURCES
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipantStatisticsListener.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipantStatisticsListener.hpp
        DEPENDS
        dds::data-writer
        statistics::types
        time
        LINK_LIBRARIES
        PUBLIC
        fastcdr
        )

    #}}}

endif()

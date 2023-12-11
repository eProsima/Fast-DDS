// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file DomainParticipant.hpp
 *
 */

#ifndef _FASTDDS_STATISTICS_DDS_DOMAIN_DOMAINPARTICIPANT_HPP_
#define _FASTDDS_STATISTICS_DDS_DOMAIN_DOMAINPARTICIPANT_HPP_

#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastrtps/fastrtps_dll.h>
#include <fastrtps/types/TypesBase.h>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastrtps {
namespace rtps {

class ReaderProxyData;
class WriterProxyData;
} // namespace rtps
} // namespace fastrtps
namespace fastdds {
namespace statistics {

class MonitorServiceStatusData;

namespace dds {

/**
 * Class DomainParticipant: extends standard DDS DomainParticipant class to include specific methods for the Statistics
 * module
 * @ingroup STATISTICS_MODULE
 */
class DomainParticipant : public eprosima::fastdds::dds::DomainParticipant
{
    DomainParticipant() = delete;

public:

    /**
     * @brief This operation enables a Statistics DataWriter
     *
     * @param[in] topic_name Name of the topic associated to the Statistics DataWriter
     * @param[in] dwqos DataWriterQos to be set
     * @return RETCODE_UNSUPPORTED if the FASTDDS_STATISTICS CMake option has not been set,
     * RETCODE_BAD_PARAMETER if the topic name provided does not correspond to any Statistics DataWriter,
     * RETCODE_INCONSISTENT_POLICY if the DataWriterQos provided are inconsistent,
     * RETCODE_OK if the DataWriter has been created or if it has been created previously,
     * and RETCODE_ERROR otherwise
     */
    RTPS_DllAPI ReturnCode_t enable_statistics_datawriter(
            const std::string& topic_name,
            const eprosima::fastdds::dds::DataWriterQos& dwqos);

    /**
     * @brief This operation enables a Statistics DataWriter from a given profile
     *
     * @param[in] profile_name DataWriter QoS profile name
     * @param[in] topic_name Name of the statistics topic to be enabled.
     * @return RETCODE_UNSUPPORTED if the FASTDDS_STATISTICS CMake option has not been set,
     * RETCODE_BAD_PARAMETER if the topic name provided does not correspond to any Statistics DataWriter,
     * RETCODE_INCONSISTENT_POLICY if the DataWriterQos provided in profile are inconsistent,
     * RETCODE_OK if the DataWriter has been created or if it has been created previously,
     * and RETCODE_ERROR otherwise
     */
    RTPS_DllAPI ReturnCode_t enable_statistics_datawriter_with_profile(
            const std::string& profile_name,
            const std::string& topic_name);

    /**
     * @brief This operation disables a Statistics DataWriter
     * @param topic_name Name of the topic associated to the Statistics DataWriter
     * @return RETCODE_UNSUPPORTED if the FASTDDS_STATISTICS CMake option has not been set,
     * RETCODE_BAD_PARAMETER if the topic name provided does not correspond to any Statistics DataWriter,
     * RETCODE_OK if the DataWriter has been correctly deleted or does not exist,
     * and RETCODE_ERROR otherwise
     */
    RTPS_DllAPI ReturnCode_t disable_statistics_datawriter(
            const std::string& topic_name);

    /**
     * @brief This operation narrows the DDS DomainParticipant to the Statistics DomainParticipant
     * @param domain_participant Reference to the DDS DomainParticipant
     * @return Reference to the Statistics DomainParticipant if successful.
     * nullptr otherwise.
     */
    RTPS_DllAPI static DomainParticipant* narrow(
            eprosima::fastdds::dds::DomainParticipant* domain_participant);

    /**
     * @brief This operation narrows the DDS DomainParticipant to the Statistics DomainParticipant
     * @param domain_participant Constant reference to the DDS DomainParticipant
     * @return Constant reference to the Statistics DomainParticipant if successful.
     * nullptr otherwise.
     */
    RTPS_DllAPI static const DomainParticipant* narrow(
            const eprosima::fastdds::dds::DomainParticipant* domain_participant);

    /**
     * Enables the monitor service in the DomainParticipant.
     *
     * @return RETCODE_OK if the monitor service could be correctly enabled.
     * @return RETCODE_ERROR if the monitor service could not be enabled properly.
     * @return RETCODE_UNSUPPORTED if FASTDDS_STATISTICS is not enabled.
     *
     * @note Not supported yet. Currently returns RETCODE_UNSUPPORTED
     */
    RTPS_DllAPI ReturnCode_t enable_monitor_service();

    /**
     * Disables the monitor service in this DomainParticipant. Does nothing if the service was not enabled before.
     *
     * @return RETCODE_OK if the monitor service could be correctly disabled.
     * @return RETCODE_NOT_ENABLED if the monitor service was not previously enabled.
     * @return RETCODE_ERROR if the service could not be properly disabled.
     * @return RETCODE_UNSUPPORTED if FASTDDS_STATISTICS is not enabled.
     *
     * @note Not supported yet. Currently returns RETCODE_UNSUPPORTED
     */
    RTPS_DllAPI ReturnCode_t disable_monitor_service();

    /**
     * fills in the ParticipantProxyData from a MonitorService Message
     *
     * @param [out] data Proxy to fill
     * @param [in] msg MonitorService Message to get the proxy information from.
     *
     * @return RETCODE_OK if the operation succeeds.
     * @return RETCODE_ERROR if the  operation fails.
     */
    RTPS_DllAPI ReturnCode_t fill_discovery_data_from_cdr_message(
            fastrtps::rtps::ParticipantProxyData& data,
            statistics::MonitorServiceStatusData& msg);

    /**
     * fills in the WriterProxyData from a MonitorService Message
     *
     * @param [out] data Proxy to fill.
     * @param [in] msg MonitorService Message to get the proxy information from.
     *
     * @return RETCODE_OK if the operation succeeds.
     * @return RETCODE_ERROR if the  operation fails.
     */
    RTPS_DllAPI ReturnCode_t fill_discovery_data_from_cdr_message(
            fastrtps::rtps::WriterProxyData& data,
            statistics::MonitorServiceStatusData& msg);

    /**
     * fills in the ReaderProxyData from a MonitorService Message
     *
     * @param [out] data Proxy to fill.
     * @param [in] msg MonitorService Message to get the proxy information from.
     *
     * @return RETCODE_OK if the operation succeeds.
     * @return RETCODE_ERROR if the  operation fails.
     */
    RTPS_DllAPI ReturnCode_t fill_discovery_data_from_cdr_message(
            fastrtps::rtps::ReaderProxyData& data,
            statistics::MonitorServiceStatusData& msg);

    /**
     * fills in the ParticipantProxyData from a MonitorService Message
     *
     * @param [out] data Proxy to fill
     * @param [in] msg MonitorService Message to get the proxy information from.
     *
     * @return RETCODE_OK if the operation succeeds.
     * @return RETCODE_ERROR if the  operation fails.
     */
    RTPS_DllAPI ReturnCode_t fill_discovery_data_from_cdr_message(
            fastrtps::rtps::ParticipantProxyData& data,
            const statistics::MonitorServiceStatusData& msg);

    /**
     * fills in the WriterProxyData from a MonitorService Message
     *
     * @param [out] data Proxy to fill.
     * @param [in] msg MonitorService Message to get the proxy information from.
     *
     * @return RETCODE_OK if the operation succeeds.
     * @return RETCODE_ERROR if the  operation fails.
     */
    RTPS_DllAPI ReturnCode_t fill_discovery_data_from_cdr_message(
            fastrtps::rtps::WriterProxyData& data,
            const statistics::MonitorServiceStatusData& msg);

    /**
     * fills in the ReaderProxyData from a MonitorService Message
     *
     * @param [out] data Proxy to fill.
     * @param [in] msg MonitorService Message to get the proxy information from.
     *
     * @return RETCODE_OK if the operation succeeds.
     * @return RETCODE_ERROR if the  operation fails.
     */
    RTPS_DllAPI ReturnCode_t fill_discovery_data_from_cdr_message(
            fastrtps::rtps::ReaderProxyData& data,
            const statistics::MonitorServiceStatusData& msg);

};

} // namespace dds
} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif /* _FASTDDS_STATISTICS_DDS_DOMAIN_DOMAINPARTICIPANT_HPP_ */

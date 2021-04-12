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

#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastrtps/fastrtps_dll.h>
#include <fastrtps/types/TypesBase.h>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace statistics {
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
     * @param topic_name Name of the topic associated to the Statistics DataWriter
     * @param dwqos DataWriterQos to be set
     * @return RETCODE_UNSUPPORTED if the FASTDDS_STATISTICS CMake option has not been set,
     * RETCODE_BAD_PARAMETER if the topic name provided does not correspond to any Statistics DataWriter,
     * RETCODE_INCONSISTENT_POLICY if the DataWriterQos provided is inconsistent,
     * RETCODE_OK if the DataWriter has been created or if it has been created previously,
     * and RETCODE_ERROR otherwise
     */
    RTPS_DllAPI ReturnCode_t enable_statistics_datawriter(
            const std::string& topic_name,
            const eprosima::fastdds::dds::DataWriterQos& dwqos);

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
     * @brief This operation enables the statistics DomainParticipant
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t enable() override;

protected:

    /**
     * Constructor
     */
    DomainParticipant(
            const eprosima::fastdds::dds::StatusMask& mask)
        : eprosima::fastdds::dds::DomainParticipant(mask)
    {
    }

    /**
     * Auxiliary function to create the statistics builtin entities.
     * @return true if succesfully created statistics DDS entities,
     * false otherwise.
     */
    void create_statistics_builtin_entities();

    /**
     * Auxiliary function to enable statistics builtin datawriters.
     * @param topic_list string with the semicolon separated list of statistics topics.
     */
    void enable_statistics_builtin_datawriters(
            const std::string& topic_list);

    /**
     * Auxiliary function to delete the statistics builtin entities.
     */
    void delete_statistics_builtin_entities();

    eprosima::fastdds::dds::Publisher* builtin_publisher_;

    friend class eprosima::fastdds::dds::DomainParticipantFactory;
};

/* Environment variable to specify a semicolon-separated list of topic names that define the statistics DataWriters that
 * the DomainParticipant will enable when enabled itself.
 * The topic names must conform to the topic names aliases defined in @ref topic_names.hpp.
 * For the variable to take any effect the CMake option FASTDDS_STATISTICS must be enabled.
 */
const char* const FASTDDS_STATISTICS_ENVIRONMENT_VARIABLE = "FASTDDS_STATISTICS";

} // namespace dds
} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif /* _FASTDDS_STATISTICS_DDS_DOMAIN_DOMAINPARTICIPANT_HPP_ */

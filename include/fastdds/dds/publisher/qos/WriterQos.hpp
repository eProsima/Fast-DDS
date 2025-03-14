// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file WriterQos.hpp
 *
 */

#ifndef FASTDDS_DDS_PUBLISHER_QOS__WRITERQOS_HPP
#define FASTDDS_DDS_PUBLISHER_QOS__WRITERQOS_HPP

#include <fastdds/dds/core/policy/QosPolicies.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class WriterQos, containing all the possible Qos that can be set for a determined Publisher.
 * Although these values can be set and are transmitted
 * during the Endpoint Discovery Protocol, not all of the behaviour associated with them has been implemented in the library.
 * Please consult each of them to check for implementation details and default values.
 * @ingroup FASTDDS_QOS_MODULE
 */
FASTDDS_TODO_BEFORE(4, 0, "Remove this class in favor of PublicationBuiltinTopicData");
class WriterQos
{
public:

    FASTDDS_EXPORTED_API WriterQos();
    FASTDDS_EXPORTED_API virtual ~WriterQos();

    bool operator ==(
            const WriterQos& b) const
    {
        return (this->m_durability == b.m_durability) &&
               (this->m_durabilityService == b.m_durabilityService) &&
               (this->m_deadline == b.m_deadline) &&
               (this->m_latencyBudget == b.m_latencyBudget) &&
               (this->m_liveliness == b.m_liveliness) &&
               (this->m_reliability == b.m_reliability) &&
               (this->m_lifespan == b.m_lifespan) &&
               (this->m_userData == b.m_userData) &&
               (this->m_timeBasedFilter == b.m_timeBasedFilter) &&
               (this->m_ownership == b.m_ownership) &&
               (this->m_ownershipStrength == b.m_ownershipStrength) &&
               (this->m_destinationOrder == b.m_destinationOrder) &&
               (this->m_presentation == b.m_presentation) &&
               (this->m_partition == b.m_partition) &&
               (this->m_topicData == b.m_topicData) &&
               (this->m_groupData == b.m_groupData) &&
               (this->m_publishMode == b.m_publishMode) &&
               (this->m_disablePositiveACKs == b.m_disablePositiveACKs) &&
               (this->representation == b.representation) &&
               (this->data_sharing == b.data_sharing);
    }

    //!Durability Qos, implemented in the library.
    DurabilityQosPolicy m_durability;

    //!Durability Service Qos, NOT implemented in the library.
    DurabilityServiceQosPolicy m_durabilityService;

    //!Deadline Qos, implemented in the library.
    DeadlineQosPolicy m_deadline;

    //!Latency Budget Qos, NOT implemented in the library.
    LatencyBudgetQosPolicy m_latencyBudget;

    //!Liveliness Qos, implemented in the library.
    LivelinessQosPolicy m_liveliness;

    //!Reliability Qos, implemented in the library.
    ReliabilityQosPolicy m_reliability;

    //!Lifespan Qos, NOT implemented in the library.
    LifespanQosPolicy m_lifespan;

    //!UserData Qos, NOT implemented in the library.
    UserDataQosPolicy m_userData;

    //!Time Based Filter Qos, NOT implemented in the library.
    TimeBasedFilterQosPolicy m_timeBasedFilter;

    //!Ownership Qos, implemented in the library.
    OwnershipQosPolicy m_ownership;

    //!Owenership Strength Qos, implemented in the library.
    OwnershipStrengthQosPolicy m_ownershipStrength;

    //!Destination Order Qos, NOT implemented in the library.
    DestinationOrderQosPolicy m_destinationOrder;

    //!Presentation Qos, NOT implemented in the library.
    PresentationQosPolicy m_presentation;

    //!Partition Qos, implemented in the library.
    PartitionQosPolicy m_partition;

    //!Topic Data Qos, NOT implemented in the library.
    TopicDataQosPolicy m_topicData;

    //!Group Data Qos, NOT implemented in the library.
    GroupDataQosPolicy m_groupData;

    //!Publication Mode Qos, implemented in the library.
    PublishModeQosPolicy m_publishMode;

    //!Data Representation Qos, implemented in the library.
    DataRepresentationQosPolicy representation;

    //!Disable positive acks QoS, implemented in the library.
    DisablePositiveACKsQosPolicy m_disablePositiveACKs;

    //!Information for data sharing compatibility check.
    DataSharingQosPolicy data_sharing;

    //! Disable heartbeat piggyback mechanism.
    bool disable_heartbeat_piggyback = false;

    /**
     * Set Qos from another class
     * @param qos Reference from a WriterQos object.
     * @param first_time Boolean indicating whether is the first time (If not some parameters cannot be set).
     *
     * @warning The use of this class and methods is discourgaed, consider using PublicationBuiltinTopicData instead.
     */
    FASTDDS_EXPORTED_API void setQos(
            const WriterQos& qos,
            bool first_time);

    /**
     * Check if the Qos values are compatible between each other.
     * @return True if correct.
     *
     * @warning The use of this class and methods is discourgaed, consider using PublicationBuiltinTopicData instead.
     */
    FASTDDS_EXPORTED_API bool checkQos() const;

    /**
     * @warning The use of this class and methods is discourgaed, consider using PublicationBuiltinTopicData instead.
     */
    FASTDDS_EXPORTED_API bool canQosBeUpdated(
            const WriterQos& qos) const;

    /**
     * @warning The use of this class and methods is discourgaed, consider using PublicationBuiltinTopicData instead.
     */
    void clear();
};

//FASTDDS_EXPORTED_API extern const WriterQos DATAWRITER_QOS_DEFAULT;

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_DDS_PUBLISHER_QOS__WRITERQOS_HPP

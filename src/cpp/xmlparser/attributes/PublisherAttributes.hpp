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
 * @file PublisherAttributes.hpp
 */

#ifndef FASTDDS_XMLPARSER_ATTRIBUTES__PUBLISHERATTRIBUTES_HPP
#define FASTDDS_XMLPARSER_ATTRIBUTES__PUBLISHERATTRIBUTES_HPP


#include <fastdds/dds/publisher/qos/WriterQos.hpp>
#include <fastdds/rtps/attributes/ExternalLocators.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <fastdds/rtps/attributes/ResourceManagement.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/Time_t.hpp>

#include <xmlparser/attributes/TopicAttributes.hpp>

namespace eprosima {
namespace fastdds {
namespace xmlparser {

/**
 * Class PublisherAttributes, used by the user to define the attributes of a Publisher.
 * @ingroup FASTDDS_ATTRIBUTES_MODULE
 */
class PublisherAttributes
{
public:

    PublisherAttributes() = default;

    virtual ~PublisherAttributes() = default;

    bool operator ==(
            const PublisherAttributes& b) const
    {
        return (this->m_userDefinedID == b.m_userDefinedID) &&
               (this->m_entityID == b.m_entityID) &&
               (this->topic == b.topic) &&
               (this->qos == b.qos) &&
               (this->times == b.times) &&
               (this->unicastLocatorList == b.unicastLocatorList) &&
               (this->multicastLocatorList == b.multicastLocatorList) &&
               (this->remoteLocatorList == b.remoteLocatorList) &&
               (this->historyMemoryPolicy == b.historyMemoryPolicy) &&
               (this->properties == b.properties);
    }

    //! Topic Attributes for the Publisher
    fastdds::xmlparser::TopicAttributes topic;

    //! QOS for the Publisher
    dds::WriterQos qos;

    //! Writer Attributes
    rtps::WriterTimes times;

    //! Unicast locator list
    rtps::LocatorList_t unicastLocatorList;

    //! Multicast locator list
    rtps::LocatorList_t multicastLocatorList;

    //! Remote locator list
    rtps::LocatorList_t remoteLocatorList;

    //! The collection of external locators to use for communication.
    rtps::ExternalLocators external_unicast_locators;

    //! Whether locators that don't match with the announced locators should be kept.
    bool ignore_non_matching_locators = false;

    //! Underlying History memory policy
    rtps::MemoryManagementPolicy_t historyMemoryPolicy =
            rtps::MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    //! Properties
    rtps::PropertyPolicy properties;

    //! Allocation limits on the matched subscribers collections
    fastdds::ResourceLimitedContainerConfig matched_subscriber_allocation;

    /**
     * Get the user defined ID
     * @return User defined ID
     */
    inline int16_t getUserDefinedID() const
    {
        return m_userDefinedID;
    }

    /**
     * Get the entity defined ID
     * @return Entity ID
     */
    inline int16_t getEntityID() const
    {
        return m_entityID;
    }

    /**
     * Set the user defined ID
     * @param id User defined ID to be set
     */
    inline void setUserDefinedID(
            uint8_t id)
    {
        m_userDefinedID = id;
    }

    /**
     * Set the entity ID
     * @param id Entity ID to be set
     */
    inline void setEntityID(
            uint8_t id)
    {
        m_entityID = id;
    }

private:

    //! User Defined ID, used for StaticEndpointDiscovery, default value -1.
    int16_t m_userDefinedID = -1;
    //! Entity ID, if the user want to specify the EntityID of the enpoint, default value -1.
    int16_t m_entityID = -1;
};

} // namespace xmlparser
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XMLPARSER_ATTRIBUTES__PUBLISHERATTRIBUTES_HPP

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
 * @file PublisherAttributes.h
 */

#ifndef PUBLISHERATTRIBUTES_H_
#define PUBLISHERATTRIBUTES_H_

#include <fastdds/rtps/resources/ResourceManagement.h>

#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/common/Time_t.h>
#include <fastdds/rtps/attributes/WriterAttributes.h>
#include <fastdds/rtps/flowcontrol/ThroughputControllerDescriptor.h>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/qos/WriterQos.h>
#include <fastdds/rtps/attributes/PropertyPolicy.h>

namespace eprosima {
namespace fastrtps{

/**
 * Class PublisherAttributes, used by the user to define the attributes of a Publisher.
 * @ingroup FASTRTPS_ATTRIBUTES_MODULE
 */
class PublisherAttributes
{
public:
    PublisherAttributes()
        : historyMemoryPolicy(rtps::PREALLOCATED_MEMORY_MODE)
        , m_userDefinedID(-1)
        , m_entityID(-1)
    {}

    virtual ~PublisherAttributes(){}

    bool operator==(const PublisherAttributes& b) const
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

    //!Topic Attributes for the Publisher
    TopicAttributes topic;

    //!QOS for the Publisher
    WriterQos qos;

    //!Writer Attributes
    rtps::WriterTimes times;

    //!Unicast locator list
    rtps::LocatorList_t unicastLocatorList;

    //!Multicast locator list
    rtps::LocatorList_t multicastLocatorList;

    //!Remote locator list
    rtps::LocatorList_t remoteLocatorList;

    //!Throughput controller
    rtps::ThroughputControllerDescriptor throughputController;

    //!Underlying History memory policy
    rtps::MemoryManagementPolicy_t historyMemoryPolicy;

    //!Properties
    rtps::PropertyPolicy properties;
    ResourceLimitedContainerConfig matched_subscriber_allocation;

    /**
     * Get the user defined ID
     * @return User defined ID
     */
    inline int16_t getUserDefinedID() const { return m_userDefinedID; }

    /**
     * Get the entity defined ID
     * @return Entity ID
     */
    inline int16_t getEntityID() const { return m_entityID; }

    /**
     * Set the user defined ID
     * @param id User defined ID to be set
     */
    inline void setUserDefinedID(uint8_t id) { m_userDefinedID = id; }

    /**
     * Set the entity ID
     * @param id Entity ID to be set
     */
    inline void setEntityID(uint8_t id) { m_entityID = id; }
private:
    //!User Defined ID, used for StaticEndpointDiscovery, default value -1.
    int16_t m_userDefinedID;
    //!Entity ID, if the user want to specify the EntityID of the enpoint, default value -1.
    int16_t m_entityID;
};

}
} /* namespace eprosima */

#endif /* PUBLISHERATTRIBUTES_H_ */

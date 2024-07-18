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
 * @file TopicAttributes.hpp
 */

#ifndef FASTDDS_XMLPARSER_ATTRIBUTES__TOPICATTRIBUTES_HPP
#define FASTDDS_XMLPARSER_ATTRIBUTES__TOPICATTRIBUTES_HPP

#include <string>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/common/Types.hpp>

namespace eprosima {
namespace fastdds {
namespace xmlparser {

/**
 * Class TopicAttributes, used to define the attributes of the topic associated with a Publisher or Subscriber.
 * @ingroup FASTDDS_ATTRIBUTES_MODULE
 */
class TopicAttributes
{
public:

    /**
     * Default constructor
     */
    TopicAttributes()
        : topicKind(rtps::NO_KEY)
        , topicName("UNDEF")
        , topicDataType("UNDEF")
    {
    }

    //!Constructor, you need to provide the topic name and the topic data type.
    TopicAttributes(
            const char* name,
            const char* dataType,
            rtps::TopicKind_t tKind = rtps::NO_KEY)
    {
        topicKind = tKind;
        topicName = name;
        topicDataType = dataType;
    }

    virtual ~TopicAttributes()
    {
    }

    bool operator ==(
            const TopicAttributes& b) const
    {
        return (this->topicKind == b.topicKind) &&
               (this->topicName == b.topicName) &&
               (this->topicDataType == b.topicDataType) &&
               (this->historyQos == b.historyQos);
    }

    /**
     * Get the topic data type
     * @return Topic data type
     */
    const fastcdr::string_255& getTopicDataType() const
    {
        return topicDataType;
    }

    /**
     * Get the topic kind
     * @return Topic kind
     */
    rtps::TopicKind_t getTopicKind() const
    {
        return topicKind;
    }

    /**
     * Get the topic name
     * @return Topic name
     */
    const fastcdr::string_255& getTopicName() const
    {
        return topicName;
    }

    //! TopicKind_t, default value NO_KEY.
    rtps::TopicKind_t topicKind;
    //! Topic Name.
    fastcdr::string_255 topicName;
    //!Topic Data Type.
    fastcdr::string_255 topicDataType;
    //!QOS Regarding the History to be saved.
    dds::HistoryQosPolicy historyQos;
    //!QOS Regarding the resources to allocate.
    dds::ResourceLimitsQosPolicy resourceLimitsQos;
    //!Type Identifier XTYPES 1.1
    dds::TypeIdV1 type_id;
    //!Type Object XTYPES 1.1
    dds::TypeObjectV1 type;
    //!XTYPES 1.2
    dds::xtypes::TypeInformationParameter type_information;

    /**
     * Method to check whether the defined QOS are correct.
     * @return True if they are valid.
     */
    bool checkQos() const;
};

/**
 * Check if two topic attributes are not equal
 * @param t1 First instance of TopicAttributes to compare
 * @param t2 Second instance of TopicAttributes to compare
 * @return True if the instances are not equal. False if the instances are equal.
 */
bool inline operator !=(
        const TopicAttributes& t1,
        const TopicAttributes& t2)
{
    if (t1.topicKind != t2.topicKind
            || t1.topicName != t2.topicName
            || t1.topicDataType != t2.topicDataType
            || t1.historyQos.kind != t2.historyQos.kind
            || (t1.historyQos.kind == dds::KEEP_LAST_HISTORY_QOS && t1.historyQos.depth != t2.historyQos.depth))
    {
        return true;
    }
    return false;
}

} // namespace xmlparser
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XMLPARSER_ATTRIBUTES__TOPICATTRIBUTES_HPP

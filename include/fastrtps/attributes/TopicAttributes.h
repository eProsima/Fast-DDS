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
 * @file TopicAttributes.h
 */

#ifndef TOPICPARAMETERS_H_
#define TOPICPARAMETERS_H_

#include <string>

#include "../rtps/common/Types.h"
#include "../qos/QosPolicies.h"


namespace eprosima {
namespace fastrtps{

/**
 * Class TopicAttributes, used by the user to define the attributes of the topic associated with a Publisher or Subscriber.
 * @ingroup FASTRTPS_ATTRIBUTES_MODULE
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
            topicDiscoveryKind = rtps::TopicDiscoveryKind_t::NO_CHECK;
        }

        //!Constructor, you need to provide the topic name and the topic data type.
        TopicAttributes(const char* name, const char* dataType, rtps::TopicKind_t tKind= rtps::NO_KEY,
            rtps::TopicDiscoveryKind_t tDiscovery = rtps::NO_CHECK)
        {
            topicKind = tKind;
            topicDiscoveryKind = tDiscovery;
            topicName = std::string(name);
            topicDataType = std::string(dataType);
        }

        virtual ~TopicAttributes() {}

        bool operator==(const TopicAttributes& b) const
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
        const std::string& getTopicDataType() const {
            return topicDataType;
        }

        /**
        * Get the topic kind
        * @return Topic kind
        */
        rtps::TopicKind_t getTopicKind() const {
            return topicKind;
        }

        /**
        * Get the Topic discoreryKind
        * @return Topic discoreryKind
        */
        rtps::TopicDiscoveryKind_t getTopicDiscoveryKind() const {
            return topicDiscoveryKind;
        }


        /**
         * Get the topic name
         * @return Topic name
         */
        const std::string& getTopicName() const {
            return topicName;
        }

        //! TopicKind_t, default value NO_KEY.
        rtps::TopicKind_t topicKind;
        //! Topic discovery kind, default value NO_CHECK.
        rtps::TopicDiscoveryKind_t topicDiscoveryKind;
        //! Topic Name.
        std::string topicName;
        //!Topic Data Type.
        std::string topicDataType;
        //!QOS Regarding the History to be saved.
        HistoryQosPolicy historyQos;
        //!QOS Regarding the resources to allocate.
        ResourceLimitsQosPolicy resourceLimitsQos;
        //!QOS Regarding the format of the data.
        DataRepresentationQosPolicy dataRepresentationQos;
        //!QOS Regarding the consistency data to check.
        TypeConsistencyEnforcementQosPolicy typeConsistencyQos;
        //!Type Identifier
        TypeIdV1 type_id;
        //!Type Object
        TypeObjectV1 type;

        /**
         * Method to check whether the defined QOS are correct.
         * @return True if they are valid.
         */
        bool checkQos() const;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * Check if two topic attributes are not equal
 * @param t1 First instance of TopicAttributes to compare
 * @param t2 Second instance of TopicAttributes to compare
 * @return True if the instances are not equal. False if the instances are equal.
 */
bool inline operator!=(const TopicAttributes& t1, const TopicAttributes& t2)
{
    if(t1.topicKind != t2.topicKind || t1.topicDiscoveryKind != t2.topicDiscoveryKind
        || t1.topicName != t2.topicName || t1.topicDataType != t2.topicDataType
        || t1.historyQos.kind != t2.historyQos.kind
        || (t1.historyQos.kind == KEEP_LAST_HISTORY_QOS && t1.historyQos.depth != t2.historyQos.depth))
    {
        return true;
    }
    return false;
}
#endif

} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* TOPICPARAMETERS_H_ */

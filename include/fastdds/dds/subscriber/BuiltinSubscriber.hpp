// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BuiltinSubscriber.hpp
 */

#ifndef BUILTINSUBSCRIBER_HPP
#define BUILTINSUBSCRIBER_HPP

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/rtps/common/InstanceHandle.h>
#include <fastrtps/rtps/common/Guid.h>

#include <fastdds/dds/topic/ParticipantBuiltinTopicData.hpp>
#include <fastdds/dds/topic/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/dds/topic/PublicationBuiltinTopicData.hpp>
#include <fastdds/dds/topic/TopicBuiltinTopicData.hpp>

#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/qos/DataWriterQos.hpp>
#include <fastdds/dds/topic/qos/DataReaderQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include <mutex>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class BuiltinSubscriber
 *  @ingroup FASTDDS_MODULE
 */
class BuiltinSubscriber
{
public:

    /**
     * Returns the BuiltinSubscriber singleton.
     * @return The BuiltinSubscriber singleton.
     */
    RTPS_DllAPI static BuiltinSubscriber* get_instance();

    RTPS_DllAPI static bool delete_instance();

    RTPS_DllAPI ParticipantBuiltinTopicData* get_participant_data(
            const fastrtps::rtps::InstanceHandle_t& participant_handle);

    RTPS_DllAPI void add_participant_data(
            const fastrtps::rtps::InstanceHandle_t& participant_handle,
            const DomainParticipantQos& pqos);

    RTPS_DllAPI void add_participant_data(
            const fastrtps::rtps::InstanceHandle_t& participant_handle,
            const UserDataQosPolicy& user_data);

    RTPS_DllAPI void delete_participant_data(
            const fastrtps::rtps::InstanceHandle_t& participant_handle);

    RTPS_DllAPI SubscriptionBuiltinTopicData* get_subscription_data(
            const fastrtps::rtps::InstanceHandle_t& reader_handle);

    RTPS_DllAPI void add_subscription_data(
            const fastrtps::rtps::InstanceHandle_t& reader_handle,
            const fastrtps::rtps::InstanceHandle_t& participant_handle,
            std::string topic_name,
            std::string type_name,
            const SubscriberQos& sqos,
            const DataReaderQos& drqos);

    RTPS_DllAPI void add_subscription_data(
            const fastrtps::rtps::InstanceHandle_t& writer_handle,
            const SubscriptionBuiltinTopicData& data);

    RTPS_DllAPI void delete_subscription_data(
            const fastrtps::rtps::InstanceHandle_t& reader_handle);

    RTPS_DllAPI PublicationBuiltinTopicData* get_publication_data(
            const fastrtps::rtps::InstanceHandle_t& writer_handle);

    RTPS_DllAPI void add_publication_data(
            const fastrtps::rtps::InstanceHandle_t& writer_handle,
            const fastrtps::rtps::InstanceHandle_t& participant_handle,
            std::string topic_name,
            std::string type_name,
            const PublisherQos& pqos,
            const DataWriterQos& dwqos);

    RTPS_DllAPI void add_publication_data(
            const fastrtps::rtps::InstanceHandle_t& writer_handle,
            const PublicationBuiltinTopicData& data);

    RTPS_DllAPI void delete_publication_data(
            const fastrtps::rtps::InstanceHandle_t& writer_handle);

    RTPS_DllAPI TopicBuiltinTopicData* get_topic_data(
            const fastrtps::rtps::InstanceHandle_t& topic_handle);

    RTPS_DllAPI void add_topic_data(
            const fastrtps::rtps::InstanceHandle_t& topic_handle,
            std::string topic_name,
            std::string type_name,
            const TopicQos& tqos);

    RTPS_DllAPI void delete_topic_data(
            const fastrtps::rtps::InstanceHandle_t& topic_handle);

private:

    BuiltinSubscriber(){}

    virtual ~BuiltinSubscriber(){}

    std::map<fastrtps::rtps::InstanceHandle_t, ParticipantBuiltinTopicData> participant_data_;
    std::mutex part_mutex_;

    std::map<fastrtps::rtps::InstanceHandle_t, SubscriptionBuiltinTopicData> subscription_data_;
    std::mutex sub_mutex_;

    std::map<fastrtps::rtps::InstanceHandle_t, PublicationBuiltinTopicData> publication_data_;
    std::mutex pub_mutex_;

    std::map<fastrtps::rtps::InstanceHandle_t, TopicBuiltinTopicData> topic_data_;
    std::mutex topic_mutex_;

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif // BUILTINSUBSCRIBER_HPP

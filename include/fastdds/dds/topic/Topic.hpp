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
 * @file Topic.hpp
 */

#ifndef _FASTDDS_TOPIC_HPP_
#define _FASTDDS_TOPIC_HPP_

#include <dds/core/status/State.hpp>
#include <fastdds/dds/core/Entity.hpp>

#include <fastrtps/fastrtps_dll.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastrtps/types/TypesBase.h>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/dds/core/status/BaseStatus.hpp>

#include <fastdds/dds/topic/DataReader.hpp>
#include <fastdds/dds/topic/DataWriter.hpp>

#include <string>
#include <vector>

namespace dds {
namespace topic {
template<typename T>
class Topic;
} // topic
} // dds

namespace eprosima {
namespace fastdds {
namespace dds {

class TopicImpl;
class TopicListener;
class DomainParticipantImpl;

class Topic : public DomainEntity,
    public TopicDescription

{
    friend class TopicImpl;
    template<typename T>
    friend class ::dds::topic::Topic;

public:

    RTPS_DllAPI Topic(
            TopicImpl* impl,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

    RTPS_DllAPI Topic(
            DomainParticipant* dp,
            const std::string& topic_name,
            const std::string& type_name,
            const TopicQos& qos = TOPIC_QOS_DEFAULT,
            TopicListener* listener = nullptr,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

    RTPS_DllAPI fastrtps::TopicAttributes get_topic_attributes() const;

    RTPS_DllAPI fastrtps::types::ReturnCode_t get_qos(
            TopicQos& qos) const;

    RTPS_DllAPI const TopicQos& get_qos() const;

    RTPS_DllAPI fastrtps::types::ReturnCode_t set_qos(
            const TopicQos& qos);

    RTPS_DllAPI TopicListener* get_listener() const;

    RTPS_DllAPI fastrtps::types::ReturnCode_t set_listener(
            TopicListener* a_listener,
            const ::dds::core::status::StatusMask& mask);

    RTPS_DllAPI DomainParticipant* get_participant() const;

    RTPS_DllAPI std::vector<DataWriter*>* get_writers();

    RTPS_DllAPI std::vector<DataReader*>* get_readers();

    RTPS_DllAPI fastrtps::types::ReturnCode_t set_instance_handle(
            const fastrtps::rtps::InstanceHandle_t& handle);

    RTPS_DllAPI fastrtps::rtps::GUID_t get_guid() const;

private:

    TopicImpl* impl_;
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif // _FASTDDS_TOPIC_HPP_

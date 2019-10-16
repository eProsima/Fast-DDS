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

#include <fastrtps/fastrtps_dll.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastrtps/types/TypesBase.h>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/dds/core/status/BaseStatus.hpp>

#include <string>
#include <vector>

namespace eprosima {
namespace fastdds {
namespace dds {

class TopicListener;
class DomainParticipant;

class Topic : public TopicDescription
{
    TopicListener* listener_;

    TopicQos qos_;

    ::dds::core::status::StatusMask mask_;

    InconsistentTopicStatus status_;

public:

	RTPS_DllAPI fastrtps::types::ReturnCode_t get_qos(
            TopicQos& qos) const;

	RTPS_DllAPI fastrtps::types::ReturnCode_t set_qos(
            const TopicQos& qos);

	RTPS_DllAPI TopicListener* get_listener() const;

	RTPS_DllAPI fastrtps::types::ReturnCode_t set_listener(
            TopicListener* a_listener,
            const ::dds::core::status::StatusMask& mask);

	RTPS_DllAPI fastrtps::types::ReturnCode_t get_inconsistent_topic_status(
            InconsistentTopicStatus& status) const;

	RTPS_DllAPI DomainParticipant* get_participant() const override;
};

}
}
}

#endif // _FASTDDS_TOPIC_HPP_

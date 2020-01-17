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
 * @file TopicImpl.hpp
 */

#ifndef _FASTDDS_TOPICIMPL_HPP_
#define _FASTDDS_TOPICIMPL_HPP_

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
#include <mutex>

namespace eprosima {
namespace fastdds {
namespace dds {

class TopicListener;
class DomainParticipantImpl;

class TopicImpl
{
    friend class DomainParticipantImpl;

    RTPS_DllAPI TopicImpl(
            DomainParticipant* dp,
            fastrtps::TopicAttributes att,
            const TopicQos& qos,
            TopicListener* listener = nullptr);

public:

    virtual ~TopicImpl();


    RTPS_DllAPI fastrtps::TopicAttributes get_topic_attributes() const;

    RTPS_DllAPI fastrtps::TopicAttributes get_topic_attributes(
            const DataReaderQos& qos) const;

    RTPS_DllAPI fastrtps::TopicAttributes get_topic_attributes(
            const DataWriterQos& qos) const;

    RTPS_DllAPI fastrtps::types::ReturnCode_t get_qos(
            TopicQos& qos) const;

    RTPS_DllAPI const TopicQos& get_qos() const;

    RTPS_DllAPI fastrtps::types::ReturnCode_t set_qos(
            const TopicQos& qos);

    RTPS_DllAPI TopicListener* get_listener() const;

    RTPS_DllAPI fastrtps::types::ReturnCode_t set_listener(
            TopicListener* a_listener);

    RTPS_DllAPI fastrtps::types::ReturnCode_t get_inconsistent_topic_status(
            InconsistentTopicStatus& status);

    RTPS_DllAPI DomainParticipant* get_participant() const;

    RTPS_DllAPI std::vector<DataWriter*>* get_writers();

    RTPS_DllAPI std::vector<DataReader*>* get_readers();

    void new_inconsistent_topic(
            const fastrtps::rtps::InstanceHandle_t& handle);

    bool is_entity_already_checked(
            const fastrtps::rtps::InstanceHandle_t& handle);

    //! Remove all listeners in the hierarchy to allow a quiet destruction
    void disable();

    Topic* user_topic_;

private:

    TopicListener* listener_;

    TopicQos qos_;

    InconsistentTopicStatus status_;

    fastrtps::TopicAttributes topic_att_;

    DomainParticipant* participant_;

    std::vector<DataReader*> readers_;
    std::vector<DataWriter*> writers_;
    std::vector<fastrtps::rtps::InstanceHandle_t> entity_with_inconsistent_topic_;
};

} // namespace eprosima
} // namespace fastdds
} // namespace dds

#endif // _FASTDDS_TOPICIMPL_HPP_

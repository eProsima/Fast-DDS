// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Topic.cpp
 *
 */

#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/topic/TopicImpl.hpp>

#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

Topic::Topic(
        const std::string& topic_name,
        TopicImpl* p,
        const StatusMask& mask)
    : DomainEntity(mask)
    , TopicDescription(topic_name, p->get_type()->getName())
    , impl_(p)
{
}

Topic::~Topic()
{
}

const TopicQos& Topic::get_qos() const
{
    return impl_->get_qos();
}

ReturnCode_t Topic::get_qos(
        TopicQos& qos) const
{
    qos = impl_->get_qos();
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t Topic::set_qos(
        const TopicQos& qos)
{
    return impl_->set_qos(qos);
}

const TopicListener* Topic::get_listener() const
{
    return impl_->get_listener();
}

ReturnCode_t Topic::set_listener(
        TopicListener* listener,
        const StatusMask& mask)
{
    TopicListener* value = mask.is_active(mask.inconsistent_topic()) ? listener : nullptr;
    ReturnCode_t ret_val = impl_->set_listener(value);
    if (ret_val == ReturnCode_t::RETCODE_OK)
    {
        status_mask_ = mask;
    }

    return ret_val;
}

DomainParticipant* Topic::get_participant() const
{
    return impl_->get_participant();
}

ReturnCode_t Topic::get_inconsistent_topic_status(
        InconsistentTopicStatus& status)
{
    // TODO: return impl_->get_inconsistent_topic_status(status);
    (void)status;
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */


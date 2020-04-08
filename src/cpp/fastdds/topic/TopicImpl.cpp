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

/*
 * TopicImpl.cpp
 *
 */

#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/topic/TopicImpl.hpp>
#include <fastdds/domain/DomainParticipantImpl.hpp>

#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/dds/log/Log.hpp>

#include <functional>

namespace eprosima {
namespace fastdds {
namespace dds {

TopicImpl::TopicImpl(
        DomainParticipantImpl* p,
        TypeSupport type_support,
        const TopicQos& qos,
        TopicListener* listen)
    : participant_(p)
    , type_support_(type_support)
    , qos_(&qos == &TOPIC_QOS_DEFAULT ? participant_->get_default_topic_qos() : qos)
    , listener_(listen)
    , user_topic_(nullptr)
{
}

TopicImpl::~TopicImpl()
{
    delete user_topic_;
}

const TopicQos& TopicImpl::get_qos() const
{
    return qos_;
}

ReturnCode_t TopicImpl::set_qos(
        const TopicQos& qos)
{
    if (qos.check_qos())
    {
        if (!qos.can_qos_be_updated(qos))
        {
            return ReturnCode_t::RETCODE_IMMUTABLE_POLICY;
        }
        qos_.set_qos(qos, false);
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
}

const TopicListener* TopicImpl::get_listener() const
{
    return listener_;
}

ReturnCode_t TopicImpl::set_listener(
        TopicListener* listener)
{
    listener_ = listener;
    return ReturnCode_t::RETCODE_OK;
}

DomainParticipant* TopicImpl::get_participant() const
{
    return participant_->get_participant();
}

const Topic* TopicImpl::get_topic() const
{
    return user_topic_;
}

const TypeSupport& TopicImpl::get_type() const
{
    return type_support_;
}

} // dds
} // fastdds
} // eprosima

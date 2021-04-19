// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTDDS_DOMAIN_PARTICIPANT_HPP_
#define _FASTDDS_DOMAIN_PARTICIPANT_HPP_

#include <string>

#include <gmock/gmock.h>
#include <fastdds/dds/core/Entity.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/Guid.h>
#include <fastrtps/types/TypesBase.h>

#include <fastdds/domain/DomainParticipantImpl.hpp>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastrtps {
namespace rtps {
class ResourceEvent;
} // rtps
} // fastrtps

namespace fastdds {
namespace dds {

class DomainParticipantListener;
class Publisher;
class PublisherListener;
class PublisherQos;
class Subscriber;
class SubscriberListener;
class SubscriberQos;
class TopicQos;
class TopicListener;

class DomainParticipant : public Entity
{
public:

    MOCK_METHOD0(delete_topic_mock, bool());

    virtual ~DomainParticipant()
    {
    }

    const DomainParticipantQos& get_qos() const
    {
        return impl_->get_qos();
    }

    const DomainParticipantListener* get_listener() const
    {
        return impl_->get_listener();
    }

    ReturnCode_t set_listener(
            DomainParticipantListener* listener)
    {
        return impl_->set_listener(listener);
    }

    ReturnCode_t enable() override
    {
        if (enable_)
        {
            return ReturnCode_t::RETCODE_OK;
        }

        enable_ = true;
        ReturnCode_t ret_code = impl_->enable();
        enable_ = !!ret_code;
        return ret_code;
    }

    Publisher* create_publisher(
            const PublisherQos& qos,
            PublisherListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all())
    {
        return impl_->create_publisher(qos, listener, mask);
    }

    Subscriber* create_subscriber(
            const SubscriberQos& qos,
            SubscriberListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all())
    {
        return impl_->create_subscriber(qos, listener, mask);
    }

    Topic* create_topic(
            const std::string& topic_name,
            const std::string& type_name,
            const TopicQos& qos,
            TopicListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all())
    {
        return impl_->create_topic(topic_name, type_name, qos, listener, mask);
    }

    ReturnCode_t delete_topic(
            const Topic* topic)
    {
        if (delete_topic_mock())
        {
            return ReturnCode_t::RETCODE_ERROR;
        }
        return impl_->delete_topic(topic);
    }

    TopicDescription* lookup_topicdescription(
            const std::string& topic_name) const
    {
        return impl_->lookup_topicdescription(topic_name);
    }

    DomainId_t get_domain_id() const
    {
        return impl_->get_domain_id();
    }

    ReturnCode_t register_type(
            TypeSupport type,
            const std::string& type_name)
    {
        return impl_->register_type(type, type_name);
    }

    ReturnCode_t register_type(
            TypeSupport type)
    {
        return impl_->register_type(type, type.get_type_name());
    }

    ReturnCode_t unregister_type(
            const std::string& typeName)
    {
        return impl_->unregister_type(typeName);
    }

    TypeSupport find_type(
            const std::string& type_name) const
    {
        return impl_->find_type(type_name);
    }

    const fastrtps::rtps::GUID_t& guid() const
    {
        return impl_->guid();
    }

    fastrtps::rtps::ResourceEvent& get_resource_event() const
    {
        return impl_->get_resource_event();
    }

    bool has_active_entities()
    {
        return impl_->has_active_entities();
    }

protected:

    DomainParticipant(
            const StatusMask& mask = StatusMask::all())
        : Entity(mask)
        , impl_(nullptr)
    {
    }

    DomainParticipantImpl* impl_;

    friend class DomainParticipantImpl;
};

} // dds
} // fastdds
} // eprosima

#endif /* _FASTDDS_DOMAIN_PARTICIPANT_HPP_ */

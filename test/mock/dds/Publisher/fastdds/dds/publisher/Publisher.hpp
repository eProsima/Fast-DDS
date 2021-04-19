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

#ifndef _FASTDDS_PUBLISHER_HPP_
#define _FASTDDS_PUBLISHER_HPP_

#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <fastdds/dds/core/Entity.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/rtps/common/InstanceHandle.h>
#include <fastrtps/types/TypesBase.h>

#include <fastdds/publisher/PublisherImpl.hpp>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipant;
class PublisherListener;
class DataWriter;
class DataWriterListener;
class Topic;

class Publisher : public Entity
{
protected:

    friend class PublisherImpl;
    friend class DomainParticipantImpl;

    Publisher(
            PublisherImpl* /*p*/,
            const StatusMask& /*mask*/)
    {
    }

    Publisher(
            DomainParticipant* /*dp*/,
            const PublisherQos& /*qos*/,
            PublisherListener* /*listener*/,
            const StatusMask& /*mask*/)
    {
    }

public:

    MOCK_METHOD0(create_datawriter_mock, bool());

    MOCK_METHOD1(delete_datawriter, ReturnCode_t(
            const DataWriter* writer));

    virtual ~Publisher()
    {
    }

    ReturnCode_t enable() override
    {
        return ReturnCode_t::RETCODE_OK;
    }

    const PublisherQos& get_qos() const
    {
        return impl_->get_qos();
    }

    ReturnCode_t set_listener(
            PublisherListener* /*listener*/)
    {
        return ReturnCode_t::RETCODE_OK;
    }

    DataWriter* create_datawriter(
        Topic* topic,
        const DataWriterQos& qos,
        DataWriterListener* listener = nullptr,
        const StatusMask& mask = StatusMask::all())
    {
        bool ret = create_datawriter_mock();
        if (ret)
        {
            return nullptr;
        }
        return impl_->create_datawriter(topic, qos, listener, mask);
    }

    DataWriter* lookup_datawriter(
            const std::string& topic_name) const
    {
        return impl_->lookup_datawriter(topic_name);
    }

    const DomainParticipant* get_participant() const
    {
        return nullptr;
    }

    const InstanceHandle_t& get_instance_handle() const
    {
        return impl_->get_instance_handle();
    }

    bool get_datawriters(
            std::vector<DataWriter*>& /*writers*/) const
    {
        return false;
    }

    bool has_datawriters() const
    {
        return false;
    }

protected:

    PublisherImpl* impl_;
};

} // dds
} // fastdds
} // eprosima

#endif /* _FASTDDS_PUBLISHER_HPP_ */

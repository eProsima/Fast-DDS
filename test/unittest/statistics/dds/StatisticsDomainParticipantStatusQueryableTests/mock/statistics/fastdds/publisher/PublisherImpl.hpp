// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PublisherImpl.hpp
 */

#ifndef _STATISTICS_FASTDDS_PUBLISHER_PUBLISHERIMPL_HPP_
#define _STATISTICS_FASTDDS_PUBLISHER_PUBLISHERIMPL_HPP_

#include <gmock/gmock.h>

#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/PublisherListener.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/EntityId_t.hpp>
#include <fastdds/statistics/IListeners.hpp>

#include <fastdds/domain/DomainParticipantImpl.hpp>
#include <fastdds/publisher/PublisherImpl.hpp>
#include <statistics/fastdds/publisher/DataWriterImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

namespace efd = eprosima::fastdds::dds;

class PublisherImpl : public efd::PublisherImpl
{
    using BaseType = efd::PublisherImpl;

public:

    MOCK_METHOD0(create_datawriter_mock, bool());

    virtual ~PublisherImpl() = default;

    PublisherImpl(
            efd::DomainParticipantImpl* p,
            const efd::PublisherQos& qos,
            efd::PublisherListener* p_listen,
            const std::shared_ptr<IListener>& stat_listener)
        : BaseType(p, qos, p_listen)
        , statistics_listener_(stat_listener)
    {
    }

    efd::DataWriterImpl* create_datawriter_impl(
            const efd::TypeSupport& type,
            efd::Topic* topic,
            const efd::DataWriterQos& qos,
            const eprosima::fastdds::rtps::EntityId_t& entity_id)
    {
        return new DataWriterImpl(this, type, topic, qos, entity_id);
    }

    efd::DataWriterImpl* create_datawriter_impl(
            const efd::TypeSupport& type,
            efd::Topic* topic,
            const efd::DataWriterQos& qos,
            efd::DataWriterListener* listener,
            std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool) override
    {
        return new DataWriterImpl(this, type, topic, qos, listener, payload_pool, statistics_listener_);
    }

    efd::DataWriter* create_datawriter(
            efd::Topic* topic,
            efd::DataWriterImpl* impl,
            const efd::StatusMask& mask)
    {
        if (create_datawriter_mock())
        {
            return nullptr;
        }
        return BaseType::create_datawriter(topic, impl, mask);
    }

    bool insert_policy_violation(
            const fastdds::rtps::GUID_t& guid,
            const fastdds::dds::QosPolicyId_t& policy_id)
    {
        bool retcode = false;

        for (auto& writer_pair : writers_)
        {
            auto writers_in_topic = writer_pair.second;
            for (auto& writer_in_topic : writers_in_topic)
            {
                if (writer_in_topic->guid() == guid)
                {
                    fastdds::dds::PolicyMask policy_mask;
                    policy_mask.set(policy_id);
                    writer_in_topic->insert_policy_violation(policy_mask);
                }
            }
        }

        return retcode;
    }

    efd::ReturnCode_t delete_datawriters()
    {
        std::unique_lock<std::mutex> lock(mtx_writers_);

        for (auto& dwt : writers_)
        {
            for (auto& dw : dwt.second)
            {
                delete (dw->user_datawriter_);
                delete (dw);
            }
        }

        writers_.clear();

        return efd::RETCODE_OK;
    }

private:

    std::shared_ptr<IListener> statistics_listener_;
};

} // dds
} // statistics
} // fastdds
} // eprosima

#endif  // _STATISTICS_FASTDDS_PUBLISHER_PUBLISHERIMPL_HPP_

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

/**
 * @file SubscriberImpl.hpp
 */

#ifndef _STATISTICS_FASTDDS_SUBSCRIBER_SUBSCRIBER_IMPL_HPP_
#define _STATISTICS_FASTDDS_SUBSCRIBER_SUBSCRIBER_IMPL_HPP_

#include <fastdds/statistics/IListeners.hpp>

#include <fastdds/subscriber/SubscriberImpl.hpp>
#include <statistics/fastdds/subscriber/DataReaderImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

namespace efd = eprosima::fastdds::dds;

class SubscriberImpl : public efd::SubscriberImpl
{
    using BaseType = efd::SubscriberImpl;

public:

    virtual ~SubscriberImpl() = default;

    SubscriberImpl(
            efd::DomainParticipantImpl* p,
            const efd::SubscriberQos& qos,
            efd::SubscriberListener* p_listen,
            const std::shared_ptr<IListener>& stat_listener)
        : BaseType(p, qos, p_listen)
        , statistics_listener_(stat_listener)
    {
    }

    efd::DataReaderImpl* create_datareader_impl(
            const efd::TypeSupport& type,
            efd::TopicDescription* topic,
            const efd::DataReaderQos& qos,
            efd::DataReaderListener* listener,
            std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool) override
    {
        return new DataReaderImpl(this, type, topic, qos, listener, payload_pool, statistics_listener_);
    }

private:

    std::shared_ptr<IListener> statistics_listener_;
};

} // dds
} // statistics
} // fastdds
} // eprosima

#endif  // _STATISTICS_FASTDDS_SUBSCRIBER_SUBSCRIBER_IMPL_HPP_

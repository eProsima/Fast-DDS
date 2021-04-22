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

#include <fastdds/subscriber/SubscriberImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

class SubscriberImpl : public eprosima::fastdds::dds::SubscriberImpl
{
    using BaseType = eprosima::fastdds::dds::SubscriberImpl;

protected:

    friend class DomainParticipantImpl;

    SubscriberImpl(
            efd::DomainParticipantImpl* p,
            const efd::SubscriberQos& qos,
            efd::SubscriberListener* p_listen = nullptr)
        : BaseType(p, qos, p_listen)
    {
    }

public:

    virtual ~SubscriberImpl() = default;
};

} // dds
} // statistics
} // fastdds
} // eprosima

#endif  // _STATISTICS_FASTDDS_SUBSCRIBER_SUBSCRIBER_IMPL_HPP_

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
 * @file PublisherImpl.hpp
 */

#ifndef _STATISTICS_FASTDDS_PUBLISHER_PUBLISHERIMPL_HPP_
#define _STATISTICS_FASTDDS_PUBLISHER_PUBLISHERIMPL_HPP_

#include <fastdds/publisher/PublisherImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

class PublisherImpl : public eprosima::fastdds::dds::PublisherImpl
{
    using BaseType = eprosima::fastdds::dds::PublisherImpl;

protected:

    friend class DomainParticipantImpl;

    PublisherImpl(
            efd::DomainParticipantImpl* p,
            const efd::PublisherQos& qos,
            efd::PublisherListener* p_listen = nullptr)
        : BaseType(p, qos, p_listen)
    {
    }

public:

    virtual ~PublisherImpl() = default;
};

} // dds
} // statistics
} // fastdds
} // eprosima

#endif  // _STATISTICS_FASTDDS_PUBLISHER_PUBLISHERIMPL_HPP_

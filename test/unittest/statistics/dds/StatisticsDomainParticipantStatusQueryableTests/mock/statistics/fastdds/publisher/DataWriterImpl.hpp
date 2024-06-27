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
 * @file DataWriterImpl.hpp
 */

#ifndef _STATISTICS_FASTDDS_PUBLISHER_DATAWRITERIMPL_HPP_
#define _STATISTICS_FASTDDS_PUBLISHER_DATAWRITERIMPL_HPP_

#include <fastdds/statistics/IListeners.hpp>

#include <fastdds/publisher/DataWriterImpl.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>

#include <statistics/fastdds/domain/DomainParticipantImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

namespace efd = eprosima::fastdds::dds;

class DataWriterImpl : public efd::DataWriterImpl
{
    using BaseType = efd::DataWriterImpl;

public:

    virtual ~DataWriterImpl() = default;

    DataWriterImpl(
            efd::PublisherImpl* p,
            efd::TypeSupport type,
            efd::Topic* topic,
            const efd::DataWriterQos& qos,
            const eprosima::fastdds::rtps::EntityId_t& entity_id)
        : BaseType(p, type, topic, qos, entity_id, nullptr)
    {
    }

    DataWriterImpl(
            efd::PublisherImpl* p,
            efd::TypeSupport type,
            efd::Topic* topic,
            const efd::DataWriterQos& qos,
            efd::DataWriterListener* listener,
            std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool,
            std::shared_ptr<IListener> stat_listener)
        : BaseType(p, type, topic, qos, listener, payload_pool)
        , statistics_listener_(stat_listener)
    {
    }

    efd::ReturnCode_t enable() override
    {
        return efd::RETCODE_OK;
    }

    void disable() override
    {

    }

private:

    std::shared_ptr<IListener> statistics_listener_;
};

} // dds
} // statistics
} // fastdds
} // eprosima

#endif  // _STATISTICS_FASTDDS_PUBLISHER_DATAWRITERIMPL_HPP_

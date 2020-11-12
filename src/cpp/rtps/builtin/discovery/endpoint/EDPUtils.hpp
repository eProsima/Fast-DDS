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
 * @file EDPUtils.hpp
 */

#ifndef _RTPS_BUILTIN_DISCOVERY_ENDPOINT_EDPUTILS_HPP_
#define _RTPS_BUILTIN_DISCOVERY_ENDPOINT_EDPUTILS_HPP_

#include <fastdds/rtps/attributes/HistoryAttributes.h>
#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastdds/rtps/attributes/WriterAttributes.h>
#include <fastdds/rtps/common/EntityId_t.hpp>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/writer/StatefulWriter.h>

#include <rtps/history/ITopicPayloadPool.h>
#include <rtps/history/PoolConfig.h>
#include <rtps/history/TopicPayloadPoolRegistry.hpp>

#include <rtps/participant/RTPSParticipantImpl.h>

#include <memory>
#include <string>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class EDPUtils
{
public:

    using WriterHistoryPair = std::pair<StatefulWriter*, WriterHistory*>;
    using ReaderHistoryPair = std::pair<StatefulReader*, ReaderHistory*>;

    static std::shared_ptr<ITopicPayloadPool> create_payload_pool(
            const std::string& topic_name,
            const HistoryAttributes& history_attr,
            bool is_reader)
    {
        PoolConfig pool_cfg = PoolConfig::from_history_attributes(history_attr);
        auto pool = TopicPayloadPoolRegistry::get(topic_name, pool_cfg);
        pool->reserve_history(pool_cfg, is_reader);
        return pool;
    }

    static void release_payload_pool(
            std::shared_ptr<ITopicPayloadPool>& pool,
            const HistoryAttributes& history_attr,
            bool is_reader)
    {
        if (pool)
        {
            PoolConfig pool_cfg = PoolConfig::from_history_attributes(history_attr);
            pool->release_history(pool_cfg, is_reader);
            TopicPayloadPoolRegistry::release(pool);
        }
    }

    static bool create_edp_reader(
            RTPSParticipantImpl* participant,
            const std::string& topic_name,
            const EntityId_t& entity_id,
            const HistoryAttributes& history_att,
            ReaderAttributes& ratt,
            ReaderListener* listener,
            std::shared_ptr<ITopicPayloadPool>& payload_pool,
            ReaderHistoryPair& edp_reader)
    {
        RTPSReader* raux = nullptr;

        payload_pool = create_payload_pool(topic_name, history_att, true);
        edp_reader.second = new ReaderHistory(history_att);
        bool created =
                participant->createReader(&raux, ratt, payload_pool, edp_reader.second, listener, entity_id,
                        true);

        if (created)
        {
            edp_reader.first = dynamic_cast<StatefulReader*>(raux);
        }
        else
        {
            delete(edp_reader.second);
            edp_reader.second = nullptr;
            release_payload_pool(payload_pool, history_att, true);
        }

        return created;
    }

    static bool create_edp_writer(
            RTPSParticipantImpl* participant,
            const std::string& topic_name,
            const EntityId_t& entity_id,
            const HistoryAttributes& history_att,
            WriterAttributes& watt,
            WriterListener* listener,
            std::shared_ptr<ITopicPayloadPool>& payload_pool,
            WriterHistoryPair& edp_writer)
    {
        RTPSWriter* waux = nullptr;

        payload_pool = create_payload_pool(topic_name, history_att, false);
        edp_writer.second = new WriterHistory(history_att);
        bool created =
                participant->createWriter(&waux, watt, payload_pool, edp_writer.second, listener, entity_id,
                        true);

        if (created)
        {
            edp_writer.first = dynamic_cast<StatefulWriter*>(waux);
        }
        else
        {
            delete(edp_writer.second);
            edp_writer.second = nullptr;
            release_payload_pool(payload_pool, history_att, false);
        }

        return created;
    }

};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif // _RTPS_BUILTIN_DISCOVERY_ENDPOINT_EDPUTILS_HPP_

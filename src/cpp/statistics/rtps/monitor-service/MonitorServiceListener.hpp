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
 * @file MonitorServiceListener.hpp
 */

#ifndef _STATISTICS_RTPS_MONITOR_SERVICE_MONITORSERVICELISTENER_HPP_
#define _STATISTICS_RTPS_MONITOR_SERVICE_MONITORSERVICELISTENER_HPP_


#include <fastdds/rtps/writer/WriterListener.hpp>

#include "Interfaces.hpp"

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

class MonitorService;

#ifdef FASTDDS_STATISTICS

class MonitorServiceListener :
    public fastdds::rtps::WriterListener,
    public IStatusObserver,
    public IConnectionsObserver,
    public IProxyObserver
{

public:

    MonitorServiceListener(
            MonitorService* ms);

    bool on_local_entity_status_change(
            const fastdds::rtps::GUID_t& guid,
            const uint32_t& id) const override;

    bool on_local_entity_change(
            const fastdds::rtps::GUID_t& guid,
            bool is_alive) const override;

    bool on_local_entity_connections_change(
            const fastdds::rtps::GUID_t& guid) const override;

    void on_writer_matched(
            fastdds::rtps::RTPSWriter* writer,
            const fastdds::rtps::MatchingInfo& info) override;

    void on_writer_change_received_by_all(
            fastdds::rtps::RTPSWriter* writer,
            fastdds::rtps::CacheChange_t* change) override;

    void on_incompatible_qos_matching(
            const fastdds::rtps::GUID_t& local_guid,
            const fastdds::rtps::GUID_t& remote_guid,
            const fastdds::dds::PolicyMask& incompatible_qos) const override;

    void on_remote_proxy_data_removed(
            const fastdds::rtps::GUID_t& removed_proxy_guid) const override;

protected:

    MonitorService* monitor_srv_;

};

#endif // FASTDDS_STATISTICS

} // namespace rtps
} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif // _STATISTICS_RTPS_MONITOR_SERVICE_MONITORSERVICELISTENER_HPP_

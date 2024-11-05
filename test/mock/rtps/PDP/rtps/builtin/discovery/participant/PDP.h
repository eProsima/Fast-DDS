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
 * @file PDP.h
 */

#ifndef FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT__PDP_H
#define FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT__PDP_H

#include <gmock/gmock.h>

#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

#include <rtps/builtin/BuiltinProtocols.h>
#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/discovery/endpoint/EDP.h>
#include <rtps/messages/CDRMessage.hpp>
#include <utils/ProxyPool.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

struct IProxyObserver;

} // namespace rtps
} // namespace statistics
} // namespace fastdds

namespace fastdds {
namespace rtps {

class PDP
{
public:

    inline std::recursive_mutex* getMutex() const
    {
        return mutex_;
    }

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD1(init, bool(
            RTPSParticipantImpl* part));

    MOCK_METHOD2(createParticipantProxyData, ParticipantProxyData*(
            const ParticipantProxyData& p,
            const GUID_t& writer_guid));

    MOCK_METHOD0(createPDPEndpoints, bool());

    MOCK_METHOD0(get_edp, EDP*());

#ifdef FASTDDS_STATISTICS
    MOCK_METHOD0(get_proxy_observer, const fastdds::statistics::rtps::IProxyObserver*());
#endif // FASTDDS_STATISTICS

    MOCK_METHOD1(assignRemoteEndpoints, void(
            ParticipantProxyData* pdata));

    MOCK_METHOD1(removeRemoteEndpoints, void(
            const ParticipantProxyData* pdata));

    MOCK_METHOD1(get_participant_proxy_data_serialized, CDRMessage_t(
            Endianness_t endian));

    MOCK_METHOD3(addReaderProxyData, ReaderProxyData*(
            const GUID_t& reader_guid,
            GUID_t& participant_guid,
            std::function<bool(ReaderProxyData*, bool, const ParticipantProxyData&)> initializer_func));

    MOCK_METHOD3(addWriterProxyData, WriterProxyData*(
            const GUID_t& writer_guid,
            GUID_t& participant_guid,
            std::function<bool(WriterProxyData*, bool, const ParticipantProxyData&)> initializer_func));

    MOCK_METHOD3(notify_incompatible_qos_matching,
            void (const GUID_t&,
            const GUID_t&,
            const fastdds::dds::PolicyMask&));

    MOCK_METHOD2(lookupReaderProxyData, bool(
            const GUID_t& reader,
            ReaderProxyData& rdata));

    MOCK_METHOD2(lookupWriterProxyData, bool(
            const GUID_t& writer,
            WriterProxyData& wdata));

    MOCK_METHOD2(notifyAboveRemoteEndpoints, void(
            const ParticipantProxyData& pdata,
            bool notify_secure_endpoints));

    MOCK_METHOD0(ParticipantProxiesBegin, ResourceLimitedVector<ParticipantProxyData*>::const_iterator());

    MOCK_METHOD0(ParticipantProxiesEnd, ResourceLimitedVector<ParticipantProxyData*>::const_iterator());

    MOCK_METHOD(RTPSParticipantImpl*, getRTPSParticipant, (), (const));

    ProxyPool<ReaderProxyData>& get_temporary_reader_proxies_pool()
    {
        return temp_proxy_readers;
    }

    ProxyPool<WriterProxyData>& get_temporary_writer_proxies_pool()
    {
        return temp_proxy_writers;
    }

    // *INDENT-ON*

    std::recursive_mutex* mutex_;

    // temporary proxies pools
    ProxyPool<ReaderProxyData> temp_proxy_readers = {{4, 1}};
    ProxyPool<WriterProxyData> temp_proxy_writers = {{4, 1}};
};

} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT__PDP_H

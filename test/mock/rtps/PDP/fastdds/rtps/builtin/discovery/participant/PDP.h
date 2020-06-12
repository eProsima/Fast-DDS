// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PDPSimple.h
 */

#ifndef _FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_PDP_H_
#define _FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_PDP_H_

#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include <fastrtps/rtps/builtin/BuiltinProtocols.h>
#include <fastrtps/rtps/messages/CDRMessage.h>
#include <fastrtps/rtps/builtin/discovery/endpoint/EDP.h>

#include <gmock/gmock.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class PDP
{
    public:

    inline std::recursive_mutex* getMutex() const {return mutex_;}

    MOCK_METHOD1(init, bool(RTPSParticipantImpl* part));

    MOCK_METHOD2(createParticipantProxyData, ParticipantProxyData*(
            const ParticipantProxyData& p,
            const GUID_t& writer_guid));

    MOCK_METHOD0(createPDPEndpoints, bool());

    MOCK_METHOD1(assignRemoteEndpoints, void(ParticipantProxyData* pdata));

    MOCK_METHOD1(notifyAboveRemoteEndpoints, void(const ParticipantProxyData& pdata));

    MOCK_METHOD1(removeRemoteEndpoints, void(const ParticipantProxyData* pdata));

    MOCK_METHOD3(addReaderProxyData, ReaderProxyData*(
            const GUID_t& reader_guid,
            GUID_t& participant_guid,
            std::function<bool(ReaderProxyData*, bool, const ParticipantProxyData&)> initializer_func));

    MOCK_METHOD3(addWriterProxyData, WriterProxyData*(
            const GUID_t& writer_guid,
            GUID_t& participant_guid,
            std::function<bool(WriterProxyData*, bool, const ParticipantProxyData&)> initializer_func));

    MOCK_METHOD2(lookupReaderProxyData, bool(
            const GUID_t& reader,
            ReaderProxyData& rdata));

    MOCK_METHOD2(lookupWriterProxyData, bool(
            const GUID_t& writer,
            WriterProxyData& wdata));

    MOCK_METHOD0(ParticipantProxiesBegin, ResourceLimitedVector<ParticipantProxyData*>::const_iterator());

    MOCK_METHOD0(ParticipantProxiesEnd, ResourceLimitedVector<ParticipantProxyData*>::const_iterator());

    std::recursive_mutex* mutex_;
};


} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // _FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_PDPSIMPLE_H_

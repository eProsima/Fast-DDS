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
 * @file EDP.h
 */

#ifndef RTPS_BUILTIN_DISCOVERY_PARTICIPANT_EDP_H_
#define RTPS_BUILTIN_DISCOVERY_PARTICIPANT_EDP_H_

#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/BuiltinProtocols.h>
#include <fastrtps/rtps/messages/CDRMessage.h>
#include <fastrtps/rtps/builtin/discovery/endpoint/EDP.h>

#include <gmock/gmock.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class EDP
{
    public:

#if HAVE_SECURITY
        MOCK_METHOD3(pairingLaterReaderProxy, bool(const GUID_t local_writer, ParticipantProxyData& pdata,
                    ReaderProxyData& rdata));

        MOCK_METHOD3(pairingLaterWriterProxy, bool(const GUID_t local_reader, ParticipantProxyData& pdata,
                WriterProxyData& wdata));
#endif
};

} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // RTPS_BUILTIN_DISCOVERY_PARTICIPANT_EDP_H_


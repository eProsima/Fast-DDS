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

#ifndef _FASTDDS_RTPS_EDP_H_
#define _FASTDDS_RTPS_EDP_H_

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
        MOCK_METHOD3(pairing_reader_proxy_with_local_writer, bool(const GUID_t& local_writer,
                    const GUID_t& remote_participant_guid, ReaderProxyData& rdata));

        MOCK_METHOD2(pairing_remote_reader_with_local_writer_after_security, bool(const GUID_t& local_writer,
                const ReaderProxyData& remote_reader_data));

        MOCK_METHOD3(pairing_writer_proxy_with_local_reader, bool(const GUID_t& local_reader,
                    const GUID_t& remote_participant_guid, WriterProxyData& wdata));

        MOCK_METHOD2(pairing_remote_writer_with_local_reader_after_security, bool(const GUID_t& local_reader,
                const WriterProxyData& remote_writer_data));
#endif
};

} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // _FASTDDS_RTPS_EDP_H_


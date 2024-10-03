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

#ifndef FASTDDS_RTPS_BUILTIN_DISCOVERY_ENDPOINT_EDP_H
#define FASTDDS_RTPS_BUILTIN_DISCOVERY_ENDPOINT_EDP_H
#include <gmock/gmock.h>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/messages/CDRMessage.hpp>
#include <rtps/builtin/BuiltinProtocols.h>
#include <utils/ProxyPool.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class EDP
{
public:

    virtual ~EDP()
    {

    }

    virtual bool initEDP(
            eprosima::fastdds::rtps::BuiltinAttributes&)
    {
        return true;
    }

    virtual void removeRemoteEndpoints(
            eprosima::fastdds::rtps::ParticipantProxyData*)
    {

    }

    virtual bool areRemoteEndpointsMatched(
            const eprosima::fastdds::rtps::ParticipantProxyData*)
    {
        return true;
    }

    virtual bool remove_reader(
            eprosima::fastdds::rtps::RTPSReader*)
    {
        return true;
    }

    virtual bool remove_writer(
            eprosima::fastdds::rtps::RTPSWriter*)
    {
        return true;
    }

    virtual void assignRemoteEndpoints(
            const eprosima::fastdds::rtps::ParticipantProxyData&,
            bool)
    {

    }

    virtual bool process_reader_proxy_data(
            eprosima::fastdds::rtps::RTPSReader*,
            eprosima::fastdds::rtps::ReaderProxyData*)
    {
        return true;
    }

    virtual bool process_writer_proxy_data(
            eprosima::fastdds::rtps::RTPSWriter*,
            eprosima::fastdds::rtps::WriterProxyData*)
    {
        return true;
    }

    MOCK_METHOD3(unpairWriterProxy, bool(
                const GUID_t& participant_guid,
                const GUID_t& writer_guid,
                bool removed_by_lease));

    MOCK_METHOD2(unpairReaderProxy, bool(
                const GUID_t& participant_guid,
                const GUID_t& reader_guid));

#if HAVE_SECURITY
    MOCK_METHOD3(pairing_reader_proxy_with_local_writer, bool(const GUID_t& local_writer,
            const GUID_t& remote_participant_guid, ReaderProxyData & rdata));

    MOCK_METHOD2(pairing_remote_reader_with_local_writer_after_security, bool(const GUID_t& local_writer,
            const ReaderProxyData& remote_reader_data));

    MOCK_METHOD3(pairing_writer_proxy_with_local_reader, bool(const GUID_t& local_reader,
            const GUID_t& remote_participant_guid, WriterProxyData & wdata));

    MOCK_METHOD2(pairing_remote_writer_with_local_reader_after_security, bool(const GUID_t& local_reader,
            const WriterProxyData& remote_writer_data));

    virtual bool pairing_remote_writer_with_local_builtin_reader_after_security(
            const eprosima::fastdds::rtps::GUID_t&,
            const eprosima::fastdds::rtps::WriterProxyData&)
    {
        return true;
    }

    virtual bool pairing_remote_reader_with_local_builtin_writer_after_security(
            const eprosima::fastdds::rtps::GUID_t&,
            const eprosima::fastdds::rtps::ReaderProxyData&)
    {
        return true;
    }

#endif // if HAVE_SECURITY
};

} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_RTPS_BUILTIN_DISCOVERY_ENDPOINT_EDP_H

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

/*!
 * @file SecurityPluginFactory.h
 */
#ifndef _RTPS_SECURITY_SECURITYMANAGER_H_
#define _RTPS_SECURITY_SECURITYMANAGER_H_

#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>

#include <gmock/gmock.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

class SecurityManager
{
    public:

    MOCK_METHOD4(discovered_reader, bool(
            const GUID_t& writer_guid,
            const GUID_t& remote_participant,
            ReaderProxyData& remote_reader_data,
            const EndpointSecurityAttributes& security_attributes));

    MOCK_METHOD3(remove_reader, void(
            const GUID_t& writer_guid,
            const GUID_t& remote_participant,
            const GUID_t& remote_reader_guid));

    MOCK_METHOD4(discovered_writer, bool(
            const GUID_t& reader_guid,
            const GUID_t& remote_participant,
            WriterProxyData& remote_writer_guid,
            const EndpointSecurityAttributes& security_attributes));

    MOCK_METHOD3(remove_writer, void(
            const GUID_t& reader_guid,
            const GUID_t& remote_participant,
            const GUID_t& remote_writer_guid));
};

} //namespace security
} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // _RTPS_SECURITY_SECURITYMANAGER_H_

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
 * @file WriterProxyData.h
 */

#ifndef _RTPS_BUILTIN_DATA_WRITERPROXYDATA_H_
#define _RTPS_BUILTIN_DATA_WRITERPROXYDATA_H_

#include <fastrtps/rtps/common/Guid.h>

#if HAVE_SECURITY
#include <fastrtps/rtps/security/accesscontrol/EndpointSecurityAttributes.h>
#endif

namespace eprosima {
namespace fastrtps {
namespace rtps {

class WriterProxyData
{
    public:

        GUID_t guid() { return m_guid; }

#if HAVE_SECURITY
        security::EndpointSecurityAttributesMask security_attributes_ = 0UL;
        security::PluginEndpointSecurityAttributesMask plugin_security_attributes_ = 0UL;
#endif

    private:

        GUID_t m_guid;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _RTPS_BUILTIN_DATA_WRITERPROXYDATA_H_


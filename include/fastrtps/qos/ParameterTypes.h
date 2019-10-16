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
 * @file ParameterTypes.h
 */

#ifndef PARAMETERTYPES_H_
#define PARAMETERTYPES_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/dds/core/policy/ParameterTypes.hpp>

#include <fastdds/rtps/common/all_common.h>
#include <fastdds/rtps/common/Token.h>

#include <fastrtps/utils/fixed_size_string.hpp>

#if HAVE_SECURITY
#include <fastdds/rtps/security/accesscontrol/ParticipantSecurityAttributes.h>
#include <fastdds/rtps/security/accesscontrol/EndpointSecurityAttributes.h>
#endif

#include <string>
#include <vector>

namespace eprosima {
namespace fastrtps {

using ParameterId_t = fastdds::dds::ParameterId_t;
using Parameter_t = fastdds::dds::Parameter_t;
using ParameterKey_t = fastdds::dds::ParameterKey_t;
using ParameterLocator_t = fastdds::dds::ParameterLocator_t;
using ParameterString_t = fastdds::dds::ParameterString_t;
using ParameterPort_t = fastdds::dds::ParameterPort_t;
using ParameterGuid_t = fastdds::dds::ParameterGuid_t;
using ParameterProtocolVersion_t = fastdds::dds::ParameterProtocolVersion_t;
using ParameterVendorId_t = fastdds::dds::ParameterVendorId_t;
using ParameterIP4Address_t = fastdds::dds::ParameterIP4Address_t;
using ParameterBool_t = fastdds::dds::ParameterBool_t;
using ParameterStatusInfo_t = fastdds::dds::ParameterStatusInfo_t;
using ParameterCount_t = fastdds::dds::ParameterCount_t;
using ParameterEntityId_t = fastdds::dds::ParameterEntityId_t;
using ParameterTime_t = fastdds::dds::ParameterTime_t;
using ParameterBuiltinEndpointSet_t = fastdds::dds::ParameterBuiltinEndpointSet_t;
using ParameterPropertyList_t = fastdds::dds::ParameterPropertyList_t;
using ParameterSampleIdentity_t = fastdds::dds::ParameterSampleIdentity_t;
#if HAVE_SECURITY
using ParameterToken_t = fastdds::dds::ParameterToken_t;
using ParameterParticipantSecurityInfo_t = fastdds::dds::ParameterParticipantSecurityInfo_t;
using ParameterEndpointSecurityInfo_t = fastdds::dds::ParameterEndpointSecurityInfo_t;
#endif

} //end of namespace
} //end of namespace eprosima

#endif
#endif /* PARAMETERTYPES_H_ */

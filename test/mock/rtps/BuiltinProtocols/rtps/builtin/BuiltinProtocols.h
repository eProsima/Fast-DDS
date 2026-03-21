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
 * @file BuiltinProtocols.h
 */

#ifndef FASTDDS_RTPS_BUILTIN__BUILTINPROTOCOLS_H
#define FASTDDS_RTPS_BUILTIN__BUILTINPROTOCOLS_H

#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/common/Locator.hpp>

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSParticipantImpl;
class WLP;

class BuiltinProtocols
{
public:

    BuiltinProtocols() = default;

    virtual ~BuiltinProtocols() = default;

    //!Locator list for metatraffic
    LocatorList_t m_metatrafficMulticastLocatorList;
    //!Locator List for metatraffic unicast
    LocatorList_t m_metatrafficUnicastLocatorList;

    LocatorList_t m_initialPeersList;

    //!BuiltinAttributes of the builtin protocols.
    BuiltinAttributes m_att;

    //! Known discovery and backup server container
    LocatorList_t m_DiscoveryServers;

    //!Pointer to the RTPSParticipantImpl.
    RTPSParticipantImpl* mp_participantImpl {nullptr};

    //!Pointer to the PDPSimple.
    PDP* mp_PDP {nullptr};

    //!Pointer to the WLP
    WLP* mp_WLP {nullptr};

    //!Pointer to the TypeLookupManager
    fastdds::dds::builtin::TypeLookupManager* typelookup_manager_ {nullptr};

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_BUILTIN__BUILTINPROTOCOLS_H

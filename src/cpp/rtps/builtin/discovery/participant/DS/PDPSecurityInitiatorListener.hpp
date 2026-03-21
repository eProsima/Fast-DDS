// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PDPSecurityInitiatorListener.h
 *
 */

#ifndef _DS_PDP_SECURITY_INITIATOR_LISTENER_H_
#define _DS_PDP_SECURITY_INITIATOR_LISTENER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <mutex>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/discovery/participant/PDPListener.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class PDP;

/**
 * Class PDPSecurityInitiatorListener, implementation for the secure discovery server handshake initiator.
 * @ingroup DISCOVERY_MODULE
 */
class PDPSecurityInitiatorListener : public PDPListener
{

    using SecurityInitiatedCallback = std::function<void (const ParticipantProxyData& participant_data)>;

public:

    /**
     * @param parent Pointer to object creating this object
     */
    PDPSecurityInitiatorListener(
            PDP* parent,
            SecurityInitiatedCallback response_cb = [] (const ParticipantProxyData&)->void {});

    virtual ~PDPSecurityInitiatorListener() override = default;

protected:

    bool check_discovery_conditions(
            ParticipantProxyData& participant_data) override;

    void process_alive_data(
            ParticipantProxyData* old_data,
            ParticipantProxyData& new_data,
            GUID_t& writer_guid,
            RTPSReader* reader,
            std::unique_lock<std::recursive_mutex>& lock) override;

    //! What action to perform upon participant discovery
    SecurityInitiatedCallback response_cb_;
};


} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _DS_PDP_SECURITY_INITIATOR_LISTENER_H_ */

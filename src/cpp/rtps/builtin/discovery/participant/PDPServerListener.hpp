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
 * @file PDPServerListener.h
 *
 */

#ifndef _FASTDDS_RTPS_PDPSERVERLISTENER2_H_
#define _FASTDDS_RTPS_PDPSERVERLISTENER2_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <rtps/builtin/discovery/participant/PDPListener.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class PDPServer;

/**
 * Class PDPServerListener, specification used by the PDP to perform the History check when a new message is received.
 * This class is implemented in order to use the same structure than with any other RTPSReader.
 *@ingroup DISCOVERY_MODULE
 */
class PDPServerListener : public fastdds::rtps::PDPListener
{
public:

    /**
     * @param in_PDP
     */
    PDPServerListener(
            PDPServer* in_PDP);

    ~PDPServerListener() override = default;

    //!Pointer to the associated mp_SPDP;
    PDPServer* pdp_server();

    /**
     * New added cache
     * This functions must be called with the reader's mutex taken
     * @param reader
     * @param change
     */
    void on_new_cache_change_added(
            fastdds::rtps::RTPSReader* reader,
            const fastdds::rtps::CacheChange_t* const change) override;

protected:

    /**
     * Checks discovery conditions on a discovery server entity.
     * Essentially, it checks for incoming PIDS of remote proxy datas.
     * @param [in]  participant_data Remote participant data to check.
     * @param [out] participant_type_str Type of the remote participant.
     * @return A pair of booleans.
     * The first one indicates if the remote participant data is valid.
     * The second one indicates if the remote participant data is a client.
     */
    std::pair<bool, bool> check_server_discovery_conditions(
            const fastdds::rtps::ParticipantProxyData& participant_data,
            std::string& participant_type_str);
};


} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_PDPSERVERLISTENER2_H_ */

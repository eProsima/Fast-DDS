// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PDPClientListener.h
 *
 */

#ifndef _FASTDDS_RTPS_PDPCLIENTLISTENER_H_
#define _FASTDDS_RTPS_PDPCLIENTLISTENER_H_

#include <rtps/builtin/discovery/participant/PDPListener.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Class PDPClientListener used by a PDP discovery client.
 * This class is implemented in order to use the same structure than with any other RTPSReader.
 * @ingroup DISCOVERY_MODULE
 */
class PDPClientListener : public PDPListener
{

public:

    /**
     * @param parent Pointer to object creating this object
     */
    PDPClientListener(
            PDP* parent);

    virtual ~PDPClientListener() override = default;

protected:

    bool check_discovery_conditions(
            ParticipantProxyData& pdata) override;

};


} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_PDPCLIENTLISTENER_H_ */

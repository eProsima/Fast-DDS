// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTDDS_RTPS_PDPSERVERLISTENER_H_
#define _FASTDDS_RTPS_PDPSERVERLISTENER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/builtin/discovery/participant/PDPListener.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class PDPServer;

/**
 * Class PDPServerListener, specification used by the PDP to perform the History check when a new message is received.
 * This class is implemented in order to use the same structure than with any other RTPSReader.
 *@ingroup DISCOVERY_MODULE
 */
class PDPServerListener: public PDPListener
{
public:
    /**
     * @param in_PDP
     */
    PDPServerListener(PDPServer* in_PDP);

    ~PDPServerListener() override = default;

    //!Pointer to the associated mp_SPDP;
    PDPServer* parent_server_pdp_;

    /**
     * New added cache
     * @param reader
     * @param change
     */
    void onNewCacheChangeAdded(
            RTPSReader* reader,
            const CacheChange_t* const change) override;
};


} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif
#endif /* _FASTDDS_RTPS_PDPSERVERLISTENER_H_ */

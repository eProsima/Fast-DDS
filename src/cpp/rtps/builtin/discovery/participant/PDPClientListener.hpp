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

#ifndef _FASTDDS_RTPS_PDPCLIENTLISTENER2_H_
#define _FASTDDS_RTPS_PDPCLIENTLISTENER2_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/builtin/discovery/participant/PDPListener.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class PDPClient;

class PDPClientListener : public fastrtps::rtps::PDPListener
{
public:

    /**
     * @param in_PDP
     */
    PDPClientListener(
            PDPClient* in_PDP);

    ~PDPClientListener() override = default;

    //!Pointer to the associated mp_SPDP;
    PDPClient* pdp_client();

    /**
     * New added cache
     * This functions must be called with the reader's mutex taken
     * @param reader
     * @param change
     */
    void onNewCacheChangeAdded(
            fastrtps::rtps::RTPSReader* reader,
            const fastrtps::rtps::CacheChange_t* const change) override;

    std::map<fastrtps::rtps::GuidPrefix_t, fastrtps::rtps::GuidPrefix_t> persistence_guid_map;
    std::map<fastrtps::rtps::GuidPrefix_t, fastrtps::rtps::SequenceNumber_t> sn_db;
};


} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_PDPCLIENTLISTENER2_H_ */

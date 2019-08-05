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
 * @file PDPListener.h
 *
 */

#ifndef _FASTDDS_RTPS_PDPLISTENER_H_
#define _FASTDDS_RTPS_PDPLISTENER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/reader/ReaderListener.h>
#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>

#include <mutex>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class PDP;

/**
 * Class PDPListener, specification used by the PDP to perform the History check when a new message is received.
 * This class is implemented in order to use the same structure than with any other RTPSReader.
 * @ingroup DISCOVERY_MODULE
 */
class PDPListener: public ReaderListener
{

public:
    /**
     * @param parent Pointer to object creating this object
     */
    PDPListener(PDP* parent);

    virtual ~PDPListener() override = default;

    /**
    * New added cache
    * @param reader
    * @param change
    */
    void onNewCacheChangeAdded(
            RTPSReader* reader,
            const CacheChange_t* const change) override;

protected:

    /**
     * Get the key of a CacheChange_t
     * @param change Pointer to the CacheChange_t
     * @return True on success
     */
    bool get_key(CacheChange_t* change);

    //!Pointer to the associated mp_SPDP;
    PDP* parent_pdp_;

    /**
     * @brief Temporary data to avoid reallocations.
     *
     * @remarks This should be always accessed with the pdp_reader lock taken
     */
    ParticipantProxyData temp_participant_data_;
};


} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif
#endif /* _FASTDDS_RTPS_PDPLISTENER_H_ */

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
 * @file EDPServer2.h
 *
 */

#ifndef _FASTDDS_RTPS_EDPSERVER2_H_
#define _FASTDDS_RTPS_EDPSERVER2_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>
#include <fastdds/rtps/builtin/discovery/DiscoveryDataFilter.hpp>
#include <fastdds/rtps/builtin/discovery/DiscoveryDataBase.hpp>
#include <fastdds/rtps/builtin/discovery/endpoint/EDPSimple.h>
#include "../participant/PDPServer2.hpp"

// To be eventually removed together with eprosima::fastrtps
namespace aux = ::eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastdds {
namespace rtps {

class EDPServerPUBListener2;
class EDPServerSUBListener2;

/**
 * Class EDPServer, implements the Endpoint Discovery Protocol for server participants
 * Inherits from EDPSimple class.
 *@ingroup DISCOVERY_MODULE
 */
class EDPServer2 : public aux::EDPSimple
{
    friend class EDPServerPUBListener2;
    friend class EDPServerSUBListener2;

public:

    /**
     * Constructor.
     * @param p Pointer to the PDP
     * @param part Pointer to the RTPSParticipantImpl
     */
    EDPServer2(
            aux::PDP* p,
            aux::RTPSParticipantImpl* part)
        : EDPSimple(p, part)
        , edp_publications_filter_(static_cast<EDPDataFilter<DiscoveryDataBase, true>*>(&dynamic_cast<PDPServer2*>(mp_PDP)->discovery_db))
        , edp_subscriptions_filter_(static_cast<EDPDataFilter<DiscoveryDataBase, false>*>(&dynamic_cast<PDPServer2*>(mp_PDP)->discovery_db))
    {
    }

    ~EDPServer2() override
    {
    }

    /**
     * This method generates the corresponding change in the subscription writer and send it to all known remote endpoints.
     * @param reader Pointer to the Reader object.
     * @param rdata Pointer to the ReaderProxyData object.
     * @return true if correct.
     */
    bool processLocalReaderProxyData(
            aux::RTPSReader* reader,
            aux::ReaderProxyData* rdata) override;
    /**
     * This method generates the corresponding change in the publciations writer and send it to all known remote endpoints.
     * @param writer Pointer to the Writer object.
     * @param wdata Pointer to the WriterProxyData object.
     * @return true if correct.
     */
    bool processLocalWriterProxyData(
            aux::RTPSWriter* writer,
            aux::WriterProxyData* wdata) override;
    /**
     * This methods generates the change disposing of the local Reader and calls the unpairing and removal methods of the base class.
     * @param R Pointer to the RTPSReader object.
     * @return True if correct.
     */
    bool removeLocalReader(
            aux::RTPSReader* R) override;
    /**
     * This methods generates the change disposing of the local Writer and calls the unpairing and removal methods of the base class.
     * @param W Pointer to the RTPSWriter object.
     * @return True if correct.
     */
    bool removeLocalWriter(
            aux::RTPSWriter* W) override;

private:

    /**
     * Create local SEDP Endpoints based on the DiscoveryAttributes.
     * @return True if correct.
     */
    virtual bool createSEDPEndpoints() override;

    //! EDP publications writer filter
    IReaderDataFilter* edp_publications_filter_;

    //! EDP subscriptions writer filter
    IReaderDataFilter* edp_subscriptions_filter_;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_EDPSERVER2_H_ */

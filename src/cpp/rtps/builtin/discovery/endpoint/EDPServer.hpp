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
 * @file EDPServer.h
 *
 */

#ifndef _FASTDDS_RTPS_EDPSERVER2_H_
#define _FASTDDS_RTPS_EDPSERVER2_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/common/CacheChange.hpp>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/discovery/database/DiscoveryDataBase.hpp>
#include <rtps/builtin/discovery/database/DiscoveryDataFilter.hpp>
#include <rtps/builtin/discovery/endpoint/EDPSimple.h>
#include <rtps/builtin/discovery/participant/PDPServer.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class EDPServerPUBListener;
class EDPServerSUBListener;

/**
 * Class EDPServer, implements the Endpoint Discovery Protocol for server participants
 * Inherits from EDPSimple class.
 *@ingroup DISCOVERY_MODULE
 */
class EDPServer : public EDPSimple
{
    friend class EDPServerPUBListener;
    friend class EDPServerSUBListener;

public:

    /**
     * Constructor.
     * @param p Pointer to the PDP
     * @param part Pointer to the RTPSParticipantImpl
     */
    EDPServer(
            PDP* p,
            RTPSParticipantImpl* part,
            DurabilityKind_t durability_kind)
        : EDPSimple(p, part)
        , durability_(durability_kind)
    {
    }

    ~EDPServer() override
    {
    }

    //! Return the PDP reference actual type
    PDPServer* get_pdp()
    {
        return static_cast<PDPServer*>(mp_PDP);
    }

    /**
     * This method generates the corresponding change in the subscription writer and send it to all known remote endpoints.
     * @param reader Pointer to the Reader object.
     * @param rdata Pointer to the ReaderProxyData object.
     * @return true if correct.
     */
    bool process_reader_proxy_data(
            RTPSReader* reader,
            ReaderProxyData* rdata) override;
    /**
     * This method generates the corresponding change in the publciations writer and send it to all known remote endpoints.
     * @param writer Pointer to the Writer object.
     * @param wdata Pointer to the WriterProxyData object.
     * @return true if correct.
     */
    bool process_writer_proxy_data(
            RTPSWriter* writer,
            WriterProxyData* wdata) override;
    /**
     * This methods generates the change disposing of the local Reader and calls the unpairing and removal methods of the base class.
     * @param R Pointer to the RTPSReader object.
     * @return True if correct.
     */
    bool remove_reader(
            RTPSReader* R) override;
    /**
     * This methods generates the change disposing of the local Writer and calls the unpairing and removal methods of the base class.
     * @param W Pointer to the RTPSWriter object.
     * @return True if correct.
     */
    bool remove_writer(
            RTPSWriter* W) override;

    /**
     * This method removes all changes from the correct data writer history with the same identity as the one in the disposal_change
     * and adds it to the corresponding builtin EDP endpoint for forwarding it to interested remotes.
     * @param change Pointer to the CacheChange_t.
     * @param discovery_db Reference to the discovery DB.
     * @param change_guid_prefix The identity of the participant from which the change came.
     * @return True if successful.
     */
    bool process_disposal(
            CacheChange_t* disposal_change,
            ddb::DiscoveryDataBase& discovery_db,
            GuidPrefix_t& change_guid_prefix,
            bool should_publish_disposal);

    /**
     * This method removes all changes from the correct data writer history (change->witerGUID.enitity)
     * and releases the change from the correct history dependeing whether the change is from
     * this participant or remote
     * @param change Pointer to the CacheChange_t.
     * @return True if successful.
     */
    bool process_and_release_change(
            CacheChange_t* change,
            bool release_from_reader);

private:

    /**
     * Create local SEDP Endpoints based on the DiscoveryAttributes.
     * @return True if correct.
     */
    virtual bool createSEDPEndpoints() override;

    //! TRANSIENT or TRANSIENT_LOCAL durability;
    DurabilityKind_t durability_;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_EDPSERVER2_H_ */

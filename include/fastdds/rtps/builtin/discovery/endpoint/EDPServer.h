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
 * @file EDPServer.h
 *
 */

#ifndef _FASTDDS_RTPS_EDPSERVER_H_
#define _FASTDDS_RTPS_EDPSERVER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>
#include <fastdds/rtps/builtin/discovery/endpoint/EDPSimple.h>

#include <set>

namespace eprosima {
namespace fastrtps {
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

    //! Keys to wipe out from WriterHistory because its related Participants have been removed
    key_list _PUBdemises, _SUBdemises;

    //! TRANSIENT or TRANSIENT_LOCAL durability;
    DurabilityKind_t _durability;

public:

    /**
     * Constructor.
     * @param p Pointer to the PDP
     * @param part Pointer to the RTPSParticipantImpl
     * @param durability_kind the kind of persistence we want for the discovery data
     */
    EDPServer(
            PDP* p,
            RTPSParticipantImpl* part,
            DurabilityKind_t durability_kind)
        : EDPSimple(p, part)
        , _durability(durability_kind)
    {
    }

    ~EDPServer() override
    {
    }

    /**
     * This method generates the corresponding change in the subscription writer and send it to all known remote endpoints.
     * @param reader Pointer to the Reader object.
     * @param rdata Pointer to the ReaderProxyData object.
     * @return true if correct.
     */
    bool processLocalReaderProxyData(
            RTPSReader* reader,
            ReaderProxyData* rdata) override;
    /**
     * This method generates the corresponding change in the publciations writer and send it to all known remote endpoints.
     * @param writer Pointer to the Writer object.
     * @param wdata Pointer to the WriterProxyData object.
     * @return true if correct.
     */
    bool processLocalWriterProxyData(
            RTPSWriter* writer,
            WriterProxyData* wdata) override;
    /**
     * This methods generates the change disposing of the local Reader and calls the unpairing and removal methods of the base class.
     * @param R Pointer to the RTPSReader object.
     * @return True if correct.
     */
    bool removeLocalReader(
            RTPSReader* R) override;
    /**
     * This methods generates the change disposing of the local Writer and calls the unpairing and removal methods of the base class.
     * @param W Pointer to the RTPSWriter object.
     * @return True if correct.
     */
    bool removeLocalWriter(
            RTPSWriter* W) override;

    /**
     * Some History data is flagged for deferred removal till every client
     * acknowledges reception
     * @return True if trimming must be done
     */
    inline bool pendingHistoryCleaning()
    {
        return !(_PUBdemises.empty() && _SUBdemises.empty());
    }

    //! Callback to remove unnecesary WriterHistory info
    bool trimPUBWriterHistory();
    bool trimSUBWriterHistory();

    //! returns true if loading info from persistency database
    bool ongoingDeserialization();

    //! Process the info recorded in the persistence database
    void processPersistentData();

    /**
     * Trigger the participant CacheChange_t removal system
     * @return True if successfully modified WriterHistory
     */
    void removePublisherFromHistory(
            const InstanceHandle_t&);
    void removeSubscriberFromHistory(
            const InstanceHandle_t&);

    /**
     * Add participant CacheChange_ts from reader to writer
     * @return True if successfully modified WriterHistory
     */
    bool addPublisherFromHistory(
            CacheChange_t& c);

    bool addSubscriberFromHistory(
            CacheChange_t& c);

private:

    /**
     * Callback to remove unnecesary WriterHistory info common implementation
     * @return True if trim is finished
     */
    template<class ProxyCont>
    bool trimWriterHistory(
            key_list& _demises,
            StatefulWriter& writer,
            WriterHistory& history,
            ProxyCont* ParticipantProxyData::* pCont);

    //! addPublisherFromHistory and addSubscriberFromHistory common implementation
    template<class Proxy>
    bool addEndpointFromHistory(
            StatefulWriter& writer,
            WriterHistory& history,
            CacheChange_t& c);

    /**
     * Create local SEDP Endpoints based on the DiscoveryAttributes.
     * @return True if correct.
     */
    virtual bool createSEDPEndpoints() override;

};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_EDPSERVER_H_ */

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
 * @file EDPSimple.h
 *
 */

#ifndef EDPSIMPLE_H_
#define EDPSIMPLE_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "EDP.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

class StatefulReader;
class StatefulWriter;
class RTPSWriter;
class RTPSReader;
class EDPSimplePUBListener;
class EDPSimpleSUBListener;
class ReaderHistory;
class WriterHistory;


/**
 * Class EDPSimple, implements the Simple Endpoint Discovery Protocol defined in the RTPS specification.
 * Inherits from EDP class.
 *@ingroup DISCOVERY_MODULE
 */
class EDPSimple : public EDP
{
    typedef std::pair<StatefulWriter*,WriterHistory*> t_p_StatefulWriter;
    typedef std::pair<StatefulReader*,ReaderHistory*> t_p_StatefulReader;

    public:

    /**
     * Constructor.
     * @param p Pointer to the PDPSimple
     * @param part Pointer to the RTPSParticipantImpl
     */
    EDPSimple(PDPSimple* p,RTPSParticipantImpl* part);
    virtual ~EDPSimple();
    //!Discovery attributes.
    BuiltinAttributes m_discovery;
    //!Pointer to the Publications Writer (only created if indicated in the DiscoveryAtributes).
    t_p_StatefulWriter mp_PubWriter;
    //!Pointer to the Subscriptions Writer (only created if indicated in the DiscoveryAtributes).
    t_p_StatefulWriter mp_SubWriter;
    //!Pointer to the Publications Reader (only created if indicated in the DiscoveryAtributes).
    t_p_StatefulReader mp_PubReader;
    //!Pointer to the Subscriptions Reader (only created if indicated in the DiscoveryAtributes).
    t_p_StatefulReader mp_SubReader;

#if HAVE_SECURITY
    t_p_StatefulWriter sedp_builtin_publications_secure_writer_;

    t_p_StatefulReader sedp_builtin_publications_secure_reader_;

    t_p_StatefulWriter sedp_builtin_subscriptions_secure_writer_;

    t_p_StatefulReader sedp_builtin_subscriptions_secure_reader_;
#endif

    //!Pointer to the ReaderListener associated with PubReader
    EDPSimplePUBListener* mp_pubListen;
    //!Pointer to the ReaderListener associated with SubReader
    EDPSimpleSUBListener* mp_subListen;

    /**
     * Initialization method.
     * @param attributes Reference to the DiscoveryAttributes.
     * @return True if correct.
     */
    bool initEDP(BuiltinAttributes& attributes) override;
    /**
     * This method assigns the remote builtin endpoints that the remote RTPSParticipant indicates is using to our local builtin endpoints.
     * @param pdata Pointer to the RTPSParticipantProxyData object.
     */
    void assignRemoteEndpoints(const ParticipantProxyData& pdata) override;
    /**
     * Remove remote endpoints from the endpoint discovery protocol
     * @param pdata Pointer to the ParticipantProxyData to remove
     */
    void removeRemoteEndpoints(ParticipantProxyData* pdata) override;

    /**
     * This method generates the corresponding change in the subscription writer and send it to all known remote endpoints.
     * @param rdata Pointer to the ReaderProxyData object.
     * @return true if correct.
     */
    bool processLocalReaderProxyData(RTPSReader* reader, ReaderProxyData* rdata) override;
    /**
     * This method generates the corresponding change in the publciations writer and send it to all known remote endpoints.
     * @param wdata Pointer to the WriterProxyData object.
     * @return true if correct.
     */
    bool processLocalWriterProxyData(RTPSWriter* writer, WriterProxyData* wdata) override;
    /**
     * This methods generates the change disposing of the local Reader and calls the unpairing and removal methods of the base class.
     * @param R Pointer to the RTPSReader object.
     * @return True if correct.
     */
    bool removeLocalReader(RTPSReader*R) override;
    /**
     * This methods generates the change disposing of the local Writer and calls the unpairing and removal methods of the base class.
     * @param W Pointer to the RTPSWriter object.
     * @return True if correct.
     */
    bool removeLocalWriter(RTPSWriter*W) override;

    private:

    /**
     * Create local SEDP Endpoints based on the DiscoveryAttributes.
     * @return True if correct.
     */
    bool createSEDPEndpoints();

#if HAVE_SECURITY
    bool create_sedp_secure_endpoints();

    bool pairing_remote_writer_with_local_builtin_reader_after_security(const GUID_t& local_reader,
                const WriterProxyData& remote_writer_data) override;

    bool pairing_remote_reader_with_local_builtin_writer_after_security(const GUID_t& local_writer,
                const ReaderProxyData& remote_reader_data) override;
#endif

};

}
} /* namespace rtps */
} /* namespace eprosima */

#endif
#endif /* EDPSIMPLE_H_ */

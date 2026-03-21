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

#ifndef FASTDDS_RTPS_BUILTIN_DISCOVERY_ENDPOINT__EDPSIMPLE_H
#define FASTDDS_RTPS_BUILTIN_DISCOVERY_ENDPOINT__EDPSIMPLE_H

#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/builtin/discovery/endpoint/EDP.h>

#include "EDPUtils.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

class StatefulReader;
class StatefulWriter;
class RTPSWriter;
class RTPSReader;
class ReaderHistory;
class WriterHistory;
class HistoryAttributes;
class ReaderAttributes;
class WriterAttributes;
class EDPListener;
class ITopicPayloadPool;
struct CacheChange_t;

/**
 * Class EDPSimple, implements the Simple Endpoint Discovery Protocol defined in the RTPS specification.
 * Inherits from EDP class.
 *@ingroup DISCOVERY_MODULE
 */
class EDPSimple : public EDP
{
    using t_p_StatefulWriter = EDPUtils::WriterHistoryPair;
    using t_p_StatefulReader = std::pair<StatefulReader*, ReaderHistory*>;

public:

    typedef std::set<InstanceHandle_t> key_list;

    /**
     * Constructor.
     * @param p Pointer to the PDP
     * @param part Pointer to the RTPSParticipantImpl
     */
    EDPSimple(
            PDP* p,
            RTPSParticipantImpl* part);

    virtual ~EDPSimple();
    //!Discovery attributes.
    BuiltinAttributes m_discovery;
    //!Pointer to the Publications Writer (only created if indicated in the DiscoveryAtributes).
    t_p_StatefulWriter publications_writer_;
    //!Pointer to the Subscriptions Writer (only created if indicated in the DiscoveryAtributes).
    t_p_StatefulWriter subscriptions_writer_;
    //!Pointer to the Publications Reader (only created if indicated in the DiscoveryAtributes).
    t_p_StatefulReader publications_reader_;
    //!Pointer to the Subscriptions Reader (only created if indicated in the DiscoveryAtributes).
    t_p_StatefulReader subscriptions_reader_;

#if HAVE_SECURITY
    t_p_StatefulWriter publications_secure_writer_;

    t_p_StatefulReader publications_secure_reader_;

    t_p_StatefulWriter subscriptions_secure_writer_;

    t_p_StatefulReader subscriptions_secure_reader_;
#endif // if HAVE_SECURITY

    //!Pointer to the listener associated with PubReader and PubWriter.
    EDPListener* publications_listener_;

    //!Pointer to the listener associated with SubReader and SubWriter.
    EDPListener* subscriptions_listener_;

    /**
     * Initialization method.
     * @param attributes Reference to the DiscoveryAttributes.
     * @return True if correct.
     */
    bool initEDP(
            BuiltinAttributes& attributes) override;
    /**
     * This method assigns the remote builtin endpoints that the remote RTPSParticipant indicates is using to our local builtin endpoints.
     * @param pdata Pointer to the RTPSParticipantProxyData object.
     * @param assign_secure_endpoints Whether to try assigning secure endpoints
     */
    void assignRemoteEndpoints(
            const ParticipantProxyData& pdata,
            bool assign_secure_endpoints) override;
    /**
     * Remove remote endpoints from the endpoint discovery protocol
     * @param pdata Pointer to the ParticipantProxyData to remove
     */
    void removeRemoteEndpoints(
            ParticipantProxyData* pdata) override;

    //! Verify whether the given participant EDP endpoints are matched with us
    bool areRemoteEndpointsMatched(
            const ParticipantProxyData* pdata) override;

    /**
     * This method generates the corresponding change in the subscription writer and send it to all known remote endpoints.
     * @param rtps_reader Pointer to the Reader object.
     * @param rdata       Pointer to the ReaderProxyData object.
     * @return true if correct.
     */
    bool process_reader_proxy_data(
            RTPSReader* rtps_reader,
            ReaderProxyData* rdata) override;
    /**
     * This method generates the corresponding change in the publciations writer and send it to all known remote endpoints.
     * @param rtps_writer Pointer to the Writer object.
     * @param wdata       Pointer to the WriterProxyData object.
     * @return true if correct.
     */
    bool process_writer_proxy_data(
            RTPSWriter* rtps_writer,
            WriterProxyData* wdata) override;
    /**
     * This methods generates the change disposing of the local Reader and calls the unpairing and removal methods of the base class.
     * @param rtps_reader Pointer to the RTPSReader object.
     * @return True if correct.
     */
    bool remove_reader(
            RTPSReader* rtps_reader) override;
    /**
     * This methods generates the change disposing of the local Writer and calls the unpairing and removal methods of the base class.
     * @param rtps_writer Pointer to the RTPSWriter object.
     * @return True if correct.
     */
    bool remove_writer(
            RTPSWriter* rtps_writer) override;

protected:

    /**
     * Initialization of history attributes for EDP built-in readers
     *
     * @param [out] attributes History attributes to initialize
     */
    virtual void set_builtin_reader_history_attributes(
            HistoryAttributes& attributes);

    /**
     * Initialization of history attributes for EDP built-in writers
     *
     * @param [out] attributes History attributes to initialize
     */
    virtual void set_builtin_writer_history_attributes(
            HistoryAttributes& attributes);

    /**
     * Initialization of reader attributes for EDP built-in readers
     *
     * @param [out] attributes Reader attributes to initialize
     */
    virtual void set_builtin_reader_attributes(
            ReaderAttributes& attributes);

    /**
     * Initialization of writer attributes for EDP built-in writers
     *
     * @param [out] attributes Writer attributes to initialize
     */
    virtual void set_builtin_writer_attributes(
            WriterAttributes& attributes);

    /**
     * Create local SEDP Endpoints based on the DiscoveryAttributes.
     * @return True if correct.
     */
    virtual bool createSEDPEndpoints();

    /**
     * Create a cache change on a builtin writer and serialize a WriterProxyData on it.
     * @param [in] data The WriterProxyData object to be serialized.
     * @param [in] writer The writer,history pair where the change should be added.
     * @param [in] remove_same_instance Should previous changes with same key be removed?
     * @param [out] created_change Where the pointer to the created change should be returned.
     * @return false if data could not be serialized into the created change.
     */
    bool serialize_writer_proxy_data(
            const WriterProxyData& data,
            const t_p_StatefulWriter& writer,
            bool remove_same_instance,
            CacheChange_t** created_change);

    /**
     * Create a cache change on a builtin writer and serialize a ReaderProxyData on it.
     * @param [in] data The ReaderProxyData object to be serialized.
     * @param [in] writer The writer,history pair where the change should be added.
     * @param [in] remove_same_instance Should previous changes with same key be removed?
     * @param [out] created_change Where the pointer to the created change should be returned.
     * @return false if data could not be serialized into the created change.
     */
    bool serialize_reader_proxy_data(
            const ReaderProxyData& data,
            const t_p_StatefulWriter& writer,
            bool remove_same_instance,
            CacheChange_t** created_change);

    //! Process the info recorded in the persistence database
    void processPersistentData(
            t_p_StatefulReader& reader,
            t_p_StatefulWriter& writer,
            key_list& demises);

    /**
     * Get a pointer pair of the corresponding writer builtin endpoint for the entity_id
     * @param [in] entity_id The entity_id to obtain the pair from.
     * @return A pair of nullptrs if operation was unsuccessful
     */
    t_p_StatefulWriter get_builtin_writer_history_pair_by_entity(
            const EntityId_t& entity_id);

    /**
     * Get a pointer pair of the corresponding reader builtin endpoint for the entity_id.
     * If a builtin writer Entity is passed, the equivalent reader entity builtin is returned.
     * @param [in] entity_id The entity_id to obtain the pair from.
     * @return A pair of nullptrs if operation was unsuccessful
     */
    t_p_StatefulReader get_builtin_reader_history_pair_by_entity(
            const EntityId_t& entity_id);

    std::shared_ptr<ITopicPayloadPool> pub_reader_payload_pool_;
    std::shared_ptr<ITopicPayloadPool> sub_reader_payload_pool_;

#if HAVE_SECURITY
    std::shared_ptr<ITopicPayloadPool> sec_pub_reader_payload_pool_;
    std::shared_ptr<ITopicPayloadPool> sec_sub_reader_payload_pool_;
#endif // if HAVE_SECURITY

private:

    /**
     * Create a cache change on a builtin writer and serialize a ProxyData on it.
     * @param [in] data The ProxyData object to be serialized.
     * @param [in] writer The writer,history pair where the change should be added.
     * @param [in] remove_same_instance Should previous changes with same key be removed?
     * @param [out] created_change Where the pointer to the created change should be returned.
     * @return false if data could not be serialized into the created change.
     */
    template<typename ProxyData>
    bool serialize_proxy_data(
            const ProxyData& data,
            const t_p_StatefulWriter& writer,
            bool remove_same_instance,
            CacheChange_t** created_change);

#if HAVE_SECURITY
    bool create_sedp_secure_endpoints();

    bool pairing_remote_writer_with_local_builtin_reader_after_security(
            const GUID_t& local_reader,
            const WriterProxyData& remote_writer_data) override;

    bool pairing_remote_reader_with_local_builtin_writer_after_security(
            const GUID_t& local_writer,
            const ReaderProxyData& remote_reader_data) override;
#endif // if HAVE_SECURITY
};

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* FASTDDS_RTPS_BUILTIN_DISCOVERY_ENDPOINT__EDPSIMPLE_H */

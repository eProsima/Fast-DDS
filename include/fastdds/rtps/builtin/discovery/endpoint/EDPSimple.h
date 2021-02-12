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

#ifndef _FASTDDS_RTPS_EDPSIMPLE_H_
#define _FASTDDS_RTPS_EDPSIMPLE_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/builtin/discovery/endpoint/EDP.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/builtin/data/ReaderProxyData.h>

namespace eprosima {
namespace fastrtps {
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

/**
 * Class EDPSimple, implements the Simple Endpoint Discovery Protocol defined in the RTPS specification.
 * Inherits from EDP class.
 *@ingroup DISCOVERY_MODULE
 */
class EDPSimple : public EDP
{
    using t_p_StatefulWriter = std::pair<StatefulWriter*, WriterHistory*>;
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
     */
    void assignRemoteEndpoints(
            const ParticipantProxyData& pdata) override;
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

    std::shared_ptr<ITopicPayloadPool> pub_writer_payload_pool_;
    std::shared_ptr<ITopicPayloadPool> pub_reader_payload_pool_;
    std::shared_ptr<ITopicPayloadPool> sub_writer_payload_pool_;
    std::shared_ptr<ITopicPayloadPool> sub_reader_payload_pool_;

#if HAVE_SECURITY
    std::shared_ptr<ITopicPayloadPool> sec_pub_writer_payload_pool_;
    std::shared_ptr<ITopicPayloadPool> sec_pub_reader_payload_pool_;
    std::shared_ptr<ITopicPayloadPool> sec_sub_writer_payload_pool_;
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

protected:

    std::mutex temp_data_lock_;
    ReaderProxyData temp_reader_proxy_data_;
    WriterProxyData temp_writer_proxy_data_;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_EDPSIMPLE_H_ */

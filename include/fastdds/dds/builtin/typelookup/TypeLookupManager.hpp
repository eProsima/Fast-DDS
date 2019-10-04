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
 * @file TypeLookupManager.hpp
 *
 */

#ifndef _FASTDDS_TYPELOOKUP_SERVICE_MANAGER_HPP
#define _FASTDDS_TYPELOOKUP_SERVICE_MANAGER_HPP
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include <vector>

#include <fastdds/dds/builtin/typelookup/TypeLookupRequestListener.hpp>
#include <fastdds/dds/builtin/typelookup/TypeLookupReplyListener.hpp>
#include <fastdds/dds/builtin/typelookup/common/TypeLookupTypes.hpp>

#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/builtin/data/ReaderProxyData.h>

namespace eprosima {
namespace fastrtps{

namespace rtps {

class BuiltinProtocols;
class ReaderHistory;
class RTPSParticipantImpl;
class StatefulReader;
class StatefulWriter;
class ParticipantProxyData;
class WriterHistory;

} // namespace rtps
} // namespace fastrtps

namespace fastdds {
namespace dds {
namespace builtin {

extern const fastrtps::rtps::SampleIdentity INVALID_SAMPLE_IDENTITY;

/**
 * Class TypeLookupManager that implements the TypeLookup Service described in the DDS-XTYPES 1.2 specification.
 * @ingroup XTYPES
 */
class TypeLookupManager
{
    friend class TypeLookupRequestListener;
    friend class TypeLookupReplyListener;

public:
    /**
    * Constructor
    * @param prot Pointer to the BuiltinProtocols object.
    */
    TypeLookupManager(
            fastrtps::rtps::BuiltinProtocols* prot);

    virtual ~TypeLookupManager();

    /**
     * Initialize the TypeLookupManager protocol.
     * @param p Pointer to the RTPS participant implementation.
     * @return true if the initialziacion was succesful.
     */
    bool init_typelookup_service(
            fastrtps::rtps::RTPSParticipantImpl* p);

    /**
     * Assign the remote endpoints for a newly discovered RTPSParticipant.
     * @param pdata Pointer to the RTPSParticipantProxyData object.
     * @return True if correct.
     */
    bool assign_remote_endpoints(
            const fastrtps::rtps::ParticipantProxyData& pdata);

    /**
     * Remove remote endpoints from the typelookup service.
     * @param pdata Pointer to the ParticipantProxyData to remove
     */
    void remove_remote_endpoints(
            fastrtps::rtps::ParticipantProxyData* pdata);

    /**
     * Get the builtin protocols
     * @return Builtin protocols
     */
    fastrtps::rtps::BuiltinProtocols* get_builtin_protocols()
    {
        return builtin_protocols_;
    }

    /**
     * Get the builtin request writer
     * @return stateful writer
     */
    fastrtps::rtps::StatefulWriter* get_builtin_request_writer();

    /**
     * Get the builtin reply writer
     * @return stateful writer
     */
    fastrtps::rtps::StatefulWriter* get_builtin_reply_writer();

    /**
    * Get the builtin request writer's history
    * @return writer history
    */
    fastrtps::rtps::WriterHistory* get_builtin_request_writer_history();

    /**
    * Get the builtin reply writer's history
    * @return writer history
    */
    fastrtps::rtps::WriterHistory* get_builtin_reply_writer_history();

    /**
     * Get the builtin request reader
     * @return stateful reader
     */
    fastrtps::rtps::StatefulReader* get_builtin_request_reader();

    /**
     * Get the builtin reply reader
     * @return stateful reader
     */
    fastrtps::rtps::StatefulReader* get_builtin_reply_reader();

    /**
    * Get the builtin request reader's history
    * @return reader history
    */
    fastrtps::rtps::ReaderHistory* get_builtin_request_reader_history();

    /**
    * Get the builtin reply reader's history
    * @return reader history
    */
    fastrtps::rtps::ReaderHistory* get_builtin_reply_reader_history();

/* TODO Uncomment if security is implemented.
#if HAVE_SECURITY
    bool pairing_remote_reader_with_local_writer_after_security(
            const fastrtps::rtps::GUID_t& local_writer,
            const fastrtps::rtps::ReaderProxyData& remote_reader_data);

    bool pairing_remote_writer_with_local_reader_after_security(
            const fastrtps::rtps::GUID_t& local_reader,
            const fastrtps::rtps::WriterProxyData& remote_writer_data);
#endif
*/

    fastrtps::rtps::SampleIdentity get_type_dependencies(
            const fastrtps::types::TypeIdentifierSeq& in) const;

    fastrtps::rtps::SampleIdentity get_types(
            const fastrtps::types::TypeIdentifierSeq& in) const;

private:
    /**
     * Create the endpoints used in the TypeLookupManager.
     * @return true if correct.
     */
    bool create_endpoints();

    /**
     * Get the RTPS participant
     * @return RTPS participant
     */
    inline fastrtps::rtps::RTPSParticipantImpl* get_RTPS_participant()
    {
        return participant_;
    }

    //! Get out instanceName as defined in 7.6.2.3.4 of the XTypes 1.2 document
    std::string get_instanceName() const;

    //! Aux method to send requests
    bool send_request(
            TypeLookup_Request& req) const;

    //! Aux method to send replies
    bool send_reply(
            TypeLookup_Reply& rep) const;

    //! Aux method to received requests
    bool recv_request(
            fastrtps::rtps::CacheChange_t& change,
            TypeLookup_Request& req) const;

    //! Aux method to received replies
    bool recv_reply(
            fastrtps::rtps::CacheChange_t& change,
            TypeLookup_Reply& rep) const;

    const fastrtps::rtps::GUID_t& get_builtin_request_writer_guid() const;

    //!Pointer to the local RTPSParticipant.
    fastrtps::rtps::RTPSParticipantImpl* participant_;

    //!Pointer to the builtinprotocol class.
    fastrtps::rtps::BuiltinProtocols* builtin_protocols_;

    //!Pointer to the builtinRTPSParticipantMEssageWriter.
    fastrtps::rtps::StatefulWriter* builtin_request_writer_;

    //!Pointer to the builtinRTPSParticipantMEssageReader.
    fastrtps::rtps::StatefulReader* builtin_request_reader_;

    //!Pointer to the builtinRTPSParticipantMEssageWriter.
    fastrtps::rtps::StatefulWriter* builtin_reply_writer_;

    //!Pointer to the builtinRTPSParticipantMEssageReader.
    fastrtps::rtps::StatefulReader* builtin_reply_reader_;

    //!Writer History
    fastrtps::rtps::WriterHistory* builtin_request_writer_history_;

    //!Writer History
    fastrtps::rtps::WriterHistory* builtin_reply_writer_history_;

    //!Reader History
    fastrtps::rtps::ReaderHistory* builtin_request_reader_history_;

    //!Reader History
    fastrtps::rtps::ReaderHistory* builtin_reply_reader_history_;

    //!Request Listener object.
    TypeLookupRequestListener* request_listener_;

    //!Reply Listener object.
    TypeLookupReplyListener* reply_listener_;

    std::mutex temp_data_lock_;
    fastrtps::rtps::ReaderProxyData temp_reader_proxy_data_;
    fastrtps::rtps::WriterProxyData temp_writer_proxy_data_;

    mutable fastrtps::rtps::SequenceNumber_t request_seq_number_;

    mutable TypeLookup_RequestTypeSupport request_type_;

    mutable TypeLookup_ReplyTypeSupport reply_type_;

/* TODO Uncomment if security is implemented.
#if HAVE_SECURITY
    //!Pointer to the builtinRTPSParticipantMEssageWriter.
    fastrtps::rtps::StatefulWriter* builtin_request_writer_secure_;

    //!Pointer to the builtinRTPSParticipantMEssageWriter.
    fastrtps::rtps::StatefulWriter* builtin_reply_writer_secure_;

    //!Pointer to the builtinRTPSParticipantMEssageReader.
    fastrtps::rtps::StatefulReader* builtin_request_reader_secure_;

    //!Pointer to the builtinRTPSParticipantMEssageReader.
    fastrtps::rtps::StatefulReader* builtin_reply_reader_secure_;

    //!Writer History
    fastrtps::rtps::WriterHistory* builtin_request_writer_secure_history_;

    //!Writer History
    fastrtps::rtps::WriterHistory* builtin_reply_writer_secure_history_;

    //!Reader History
    fastrtps::rtps::ReaderHistory* builtin_request_reader_secure_history_;

    //!Reader History
    fastrtps::rtps::ReaderHistory* builtin_reply_reader_secure_history_;

    bool create_secure_endpoints();
#endif
*/
};

} /* namespace builtin */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif
#endif /* _FASTDDS_TYPELOOKUP_SERVICE_MANAGER_HPP */

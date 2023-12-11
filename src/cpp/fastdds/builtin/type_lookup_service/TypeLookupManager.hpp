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
#include <mutex>
#include <unordered_map>

#include <fastdds/rtps/builtin/data/ReaderProxyData.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>

#include <fastdds/builtin/type_lookup_service/detail/TypeLookupTypes.hpp>
#include <fastdds/builtin/type_lookup_service/detail/TypeLookupTypesPubSubTypes.h>
#include <fastdds/builtin/type_lookup_service/TypeLookupReplyListener.hpp>
#include <fastdds/builtin/type_lookup_service/TypeLookupRequestListener.hpp>

namespace eprosima {
namespace fastrtps {

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

using AsyncGetTypeCallback = std::function<void ()>;

inline SequenceNumber_t get_sequence_number_from_rtps(
        fastrtps::rtps::SequenceNumber_t seq_number)
{
    SequenceNumber_t dds_seq_number;
    dds_seq_number.low() = seq_number.low;
    dds_seq_number.high() = seq_number.high;

    return dds_seq_number;
}

inline GUID_t get_guid_from_rtps(
        const fastrtps::rtps::GUID_t& rtps_guid)
{
    GUID_t guid;
    std::memcpy(guid.guidPrefix().data(), rtps_guid.guidPrefix.value, 12);
    for (size_t i = 0; i < 3; i++)
    {
        guid.entityId().entityKey()[i] = rtps_guid.entityId.value[i + 1];
    }
    guid.entityId().entityKind() = rtps_guid.entityId.value[0];

    return guid;
}

inline fastrtps::rtps::GUID_t get_rtps_guid(
        const GUID_t& guid)
{
    fastrtps::rtps::GUID_t rtps_guid;
    std::memcpy(rtps_guid.guidPrefix.value, guid.guidPrefix().data(), 12);
    for (size_t i = 0; i < 3; i++)
    {
        rtps_guid.entityId.value[i + 1] = guid.entityId().entityKey()[i];
    }
    rtps_guid.entityId.value[0] = guid.entityId().entityKind();

    return rtps_guid;
}

inline fastrtps::rtps::SampleIdentity get_rtps_sample_identity(
        const SampleIdentity& sampleid)
{
    fastrtps::rtps::SampleIdentity rtps_sampleid;
    rtps_sampleid.writer_guid(get_rtps_guid(sampleid.writer_guid()));
    rtps_sampleid.sequence_number().high = sampleid.sequence_number().high();
    rtps_sampleid.sequence_number().low = sampleid.sequence_number().low();

    return rtps_sampleid;
}

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
            fastrtps::rtps::BuiltinProtocols* protocols);

    virtual ~TypeLookupManager();

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

    fastrtps::rtps::SampleIdentity get_type_dependencies(
            const fastrtps::types::TypeIdentifierSeq& id_seq) const;

    fastrtps::rtps::SampleIdentity get_types(
            const fastrtps::types::TypeIdentifierSeq& id_seq) const;

    ReturnCode_t async_get_type(
            xtypes::TypeInformation typeinformation,
            GuidPrefix_t type_server,
            AsyncGetTypeCallback& callback);

private:

    /**
     * Create the endpoints used in the TypeLookupManager.
     * @return true if correct.
     */
    bool create_endpoints();

    //! Aux method to send requests
    bool send_request(
            TypeLookup_Request& request) const;

    //! Aux method to send replies
    bool send_reply(
            TypeLookup_Reply& reply) const;

    //! Aux method to received requests
    bool recv_request(
            fastrtps::rtps::CacheChange_t& change,
            TypeLookup_Request& request) const;

    //! Aux method to received replies
    bool recv_reply(
            fastrtps::rtps::CacheChange_t& change,
            TypeLookup_Reply& reply) const;

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

    fastrtps::rtps::RTPSParticipantImpl* participant_;
    fastrtps::rtps::BuiltinProtocols* builtin_protocols_;
    fastrtps::rtps::StatefulWriter* builtin_request_writer_;
    fastrtps::rtps::StatefulReader* builtin_request_reader_;
    fastrtps::rtps::StatefulWriter* builtin_reply_writer_;
    fastrtps::rtps::StatefulReader* builtin_reply_reader_;
    fastrtps::rtps::WriterHistory* builtin_request_writer_history_;
    fastrtps::rtps::WriterHistory* builtin_reply_writer_history_;
    fastrtps::rtps::ReaderHistory* builtin_request_reader_history_;
    fastrtps::rtps::ReaderHistory* builtin_reply_reader_history_;
    TypeLookupRequestListener* request_listener_;
    TypeLookupReplyListener* reply_listener_;

    std::mutex temp_data_lock_;
    fastrtps::rtps::ReaderProxyData temp_reader_proxy_data_;
    fastrtps::rtps::WriterProxyData temp_writer_proxy_data_;

    mutable fastrtps::rtps::SequenceNumber_t request_seq_number_;
    mutable TypeLookup_RequestPubSubType request_type_;
    mutable TypeLookup_ReplyPubSubType reply_type_;
};

} /* namespace builtin */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_TYPELOOKUP_SERVICE_MANAGER_HPP */

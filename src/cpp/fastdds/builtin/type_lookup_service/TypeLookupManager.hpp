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

#include <mutex>
#include <unordered_map>
#include <vector>

#include <fastdds/rtps/builtin/data/ReaderProxyData.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>

#include <fastdds/builtin/type_lookup_service/detail/TypeLookupTypes.hpp>
#include <fastdds/builtin/type_lookup_service/detail/TypeLookupTypesPubSubTypes.h>
#include <fastdds/builtin/type_lookup_service/TypeLookupReplyListener.hpp>
#include <fastdds/builtin/type_lookup_service/TypeLookupRequestListener.hpp>
#include <fastdds/xtypes/type_representation/TypeIdentifierWithSizeHashSpecialization.h>
#include <fastrtps/utils/ProxyPool.hpp>

namespace std {

template<>
struct hash<eprosima::fastdds::dds::xtypes::TypeInformation>
{
    std::size_t operator ()(
            const eprosima::fastdds::dds::xtypes::TypeInformation& k) const
    {
        return (static_cast<size_t>(k.complete().typeid_with_size().type_id().equivalence_hash()[0]) << 16) |
               (static_cast<size_t>(k.complete().typeid_with_size().type_id().equivalence_hash()[1]) << 8) |
               (static_cast<size_t>(k.complete().typeid_with_size().type_id().equivalence_hash()[2]));
    }

};

template <>
struct hash<eprosima::fastdds::dds::SampleIdentity>
{
    std::size_t operator ()(
            const eprosima::fastdds::dds::SampleIdentity& k) const
    {
        std::size_t hash_value = 0;

        // Hash m_writer_guid
        for (const auto& byte : k.writer_guid().guidPrefix())
        {
            hash_value ^= std::hash<uint8_t>{}(byte);
        }

        hash_value ^= std::hash<uint8_t>{}(k.writer_guid().entityId().entityKind());

        // Hash m_entityKey of m_writer_guid's EntityId_t
        for (const auto& byte : k.writer_guid().entityId().entityKey())
        {
            hash_value ^= std::hash<uint8_t>{}(byte);
        }

        // Hash m_sequence_number
        hash_value ^= std::hash<int32_t>{}(k.sequence_number().high());
        hash_value ^= std::hash<uint32_t>{}(k.sequence_number().low());

        return hash_value;
    }

};

} // std

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

const SampleIdentity INVALID_SAMPLE_IDENTITY;

using AsyncGetTypeWriterCallback = std::function<
    void (eprosima::ProxyPool<eprosima::fastrtps::rtps::WriterProxyData>::smart_ptr && temp_writer_data)>;
using AsyncGetTypeReaderCallback = std::function<
    void (eprosima::ProxyPool<eprosima::fastrtps::rtps::ReaderProxyData>::smart_ptr && temp_reader_data)>;

/**
 * Class TypeLookupManager that implements the TypeLookup Service described in the DDS-XTYPES 1.3 specification.
 * @ingroup XTYPES
 */
class TypeLookupManager
{
    friend class TypeLookupRequestListener;
    friend class TypeLookupReplyListener;

public:

    /**
     * Constructor
     */
    TypeLookupManager();

    virtual ~TypeLookupManager();

    /**
     * Stores pointers to the RTPSParticipantImpl and BuiltinProtocols and creates temp
     * ReaderProxyData and WriterProxyData objects for the TypeLookupManager.
     * @param prot Pointer to the BuiltinProtocols object.
     * @return true if members and endpoints are created, false otherwise.
     */
    bool init(
            fastrtps::rtps::BuiltinProtocols* protocols);
    /**
     * Assign the remote endpoints for a newly discovered RTPSParticipant.
     * @param pdata Pointer to the RTPSParticipantProxyData object.
     * @return True if correct.
     */
    bool assign_remote_endpoints(
            const fastrtps::rtps::ParticipantProxyData& pdata);

    /**
     * Remove remote endpoints from the typelookup service.
     * @param pdata Pointer to the ParticipantProxyData to remove.
     */
    void remove_remote_endpoints(
            fastrtps::rtps::ParticipantProxyData* pdata);

    /**
     * Create and send a request using the builtin TypeLookup Service to retrieve all the type dependencies
     * associated with a sequence of TypeIdentifiers.
     * @param id_seq[in] Sequence of TypeIdentifiers for which dependencies are needed.
     * @param type_server[in] GuidPrefix corresponding to the remote participant which TypeInformation is being solved.
     * @param continuation_point[in] Continuation point for a previous partially answered request.
     * @return The SampleIdentity of the sent request.
     */
    SampleIdentity get_type_dependencies(
            const xtypes::TypeIdentifierSeq& id_seq,
            const fastrtps::rtps::GuidPrefix_t& type_server,
            const std::vector<uint8_t>& continuation_point = std::vector<uint8_t>()) const;

    /**
     * Create and send a request using the built-in TypeLookup Service to retrieve TypeObjects associated with a
     * sequence of TypeIdentifiers.
     * @param id_seq[in] Sequence of TypeIdentifiers for which TypeObjects are to be retrieved.
     * @param type_server[in] GuidPrefix corresponding to the remote participant whose TypeInformation is being resolved.
     * @return The SampleIdentity of the sent request.
     */
    SampleIdentity get_types(
            const xtypes::TypeIdentifierSeq& id_seq,
            const fastrtps::rtps::GuidPrefix_t& type_server) const;

    /**
     * Use builtin TypeLookup service to solve the type and dependencies of a given TypeInformation.
     * It receives a callback that will be used to notify when the negotiation is complete.
     * @param temp_proxy_data[in] Temporary Writer/Reader ProxyData that originated the request.
     * @param callback Callback called when the negotiation is complete.
     * @return ReturnCode_t RETCODE_OK if the type is already known.
     *                      RETCODE_NO_DATA if type is not known, and a negotiation has been started.
     *                      RETCODE_ERROR if any request was not sent correctly.
     */
    ReturnCode_t async_get_type(
            eprosima::ProxyPool<eprosima::fastrtps::rtps::WriterProxyData>::smart_ptr&& temp_proxy_data,
            const AsyncGetTypeWriterCallback& callback);
    ReturnCode_t async_get_type(
            eprosima::ProxyPool<eprosima::fastrtps::rtps::ReaderProxyData>::smart_ptr&& temp_proxy_data,
            const AsyncGetTypeReaderCallback& callback);

private:

    /**
     * Checks if the given TypeIdentfierWithSize is known by the TypeObjectRegistry.
     * Uses get_type_dependencies() and get_types() to get those that are not known.
     * Adds a callback to the async_get_type_callbacks_ entry of the TypeIdentfierWithSize, or creates a new one if
     * TypeIdentfierWithSize was not in the map before.
     * @param type_identifier_with_size[in] TypeIdentfierWithSize to check.
     * @param type_server[in] GuidPrefix corresponding to the remote participant which TypeInformation is being solved.
     * @return ReturnCode_t RETCODE_OK if type is known.
     *                      RETCODE_NO_DATA if the type is being discovered.
     *                      RETCODE_ERROR if the request was not sent or the callback was not added correctly.
     */
    ReturnCode_t check_type_identifier_received(
            const xtypes::TypeIdentfierWithSize& type_identifier_with_size,
            const fastrtps::rtps::GuidPrefix_t& type_server);
    /**
     * Checks if the given TypeIdentfierWithSize is known by the TypeObjectRegistry.
     * Uses get_type_dependencies() and get_types() to get those that are not known.
     * Adds a callback to the async_get_type_callbacks_ entry of the TypeIdentfierWithSize, or creates a new one if
     * TypeIdentfierWithSize was not in the map before.
     * @param temp_proxy_data[in] Temporary Writer/Reader ProxyData that originated the request.
     * @param callback[in] Callback to add.
     * @return ReturnCode_t RETCODE_OK if type is known.
     *                      RETCODE_NO_DATA if the type is being discovered.
     *                      RETCODE_ERROR if the request was not sent or the callback was not added correctly.
     */
    ReturnCode_t check_type_identifier_received(
            eprosima::ProxyPool<eprosima::fastrtps::rtps::WriterProxyData>::smart_ptr&& temp_writer_data,
            const AsyncGetTypeWriterCallback& callback);
    ReturnCode_t check_type_identifier_received(
            eprosima::ProxyPool<eprosima::fastrtps::rtps::ReaderProxyData>::smart_ptr&&  temp_reader_data,
            const AsyncGetTypeReaderCallback& callback);

    /**
     * Implementation for check_type_identifier_received method.
     * Checks if the given TypeIdentfierWithSize is known by the TypeObjectRegistry.
     * Uses get_type_dependencies() and get_types() to get those that are not known.
     * Adds a callback to the async_get_type_callbacks_ entry of the TypeIdentfierWithSize, or creates a new one if
     * TypeIdentfierWithSize was not in the map before
     * @param temp_proxy_data[in] Temporary Writer/Reader ProxyData that originated the request.
     * @param callback[in] Callback to add.
     * @param async_get_type_callbacks[in] The collection ProxyData and their callbacks to use.
     * @return ReturnCode_t RETCODE_OK if type is known.
     *                      RETCODE_NO_DATA if the type is being discovered.
     *                      RETCODE_ERROR if the request was not sent or the callback was not added correctly.
     */
    template <typename ProxyType, typename AsyncCallback>
    ReturnCode_t check_type_identifier_received_impl(
            typename eprosima::ProxyPool<ProxyType>::smart_ptr&& temp_proxy_data,
            const AsyncCallback& callback,
            std::unordered_map<xtypes::TypeIdentfierWithSize,
            std::vector<std::pair<typename eprosima::ProxyPool<ProxyType>::smart_ptr,
            AsyncCallback>>>& async_get_type_callbacks);

    /**
     *  Notifies callbacks for a given TypeIdentfierWithSize.
     * @param type_identifier_with_size[in] TypeIdentfierWithSize of the callbacks to notify.
     */
    void notify_callbacks(
            xtypes::TypeIdentfierWithSize type_identifier_with_size);

    /**
     * Adds a callback to the async_get_type_callbacks_ entry of the TypeIdentfierWithSize, or creates a new one if
     * TypeIdentfierWithSize was not in the map before.
     * @param request[in] SampleIdentity of the request.
     * @param type_identifier_with_size[in] TypeIdentfierWithSize that originated the request.
     * @return true if added. false otherwise.
     */
    bool add_async_get_type_request(
            const SampleIdentity& request,
            const xtypes::TypeIdentfierWithSize& type_identifier_with_size );

    /**
     * Removes a TypeIdentfierWithSize from the async_get_type_callbacks_.
     * @param type_identifier_with_size[in] TypeIdentfierWithSize to be removed.
     * @return true if removed, false otherwise.
     */
    bool remove_async_get_type_callback(
            const xtypes::TypeIdentfierWithSize& type_identifier_with_size);

    /**
     * Removes a SampleIdentity from the async_get_type_callbacks_.
     * @param request[in] SampleIdentity to be removed.
     * @return true if removed, false otherwise.
     */
    bool remove_async_get_types_request(
            SampleIdentity request);

    /**
     * Complete requests common fields, create CacheChange, serialize request and add change to writer history.
     * @param type_id[in] TypeIdentfierWithSize that originated the request.
     * @param request[in] TypeLookup_Request to be sent.
     * @return true if request was sent, false otherwise.
     */
    bool send_request(
            const fastrtps::rtps::GuidPrefix_t& type_server,
            TypeLookup_Request& request) const;

    /**
     * Complete reply common fields, create CacheChange, serialize reply and add change to writer history.
     * @param reply[in] TypeLookup_Reply to be sent.
     * @return true if reply was sent, false otherwise.
     */
    bool send_reply(
            TypeLookup_Reply& reply) const;

    /**
     * Used for request reception. Deserialize the request and check if it is directed to the local DomainParticipant.
     * @param change[in] CacheChange_t of the request.
     * @param request[out] TypeLookup_Request after deserialization.
     * @return true if the request is deserialized and directed to the local participant, false otherwise.
     */
    bool receive_request(
            fastrtps::rtps::CacheChange_t& change,
            TypeLookup_Request& request) const;

    /**
     * Used for reply reception. Deserialize and check that the reply's recipient is the local participant.
     * @param change[in] CacheChange_t of the reply.
     * @param reply[out] TypeLookup_Reply after deserialize.
     * @return true if the request is deserialized and the reply's recipient is us, false otherwise.
     */
    bool receive_reply(
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

    /**
     * Create instance name as defined in section 7.6.3.3.4 XTypes 1.3 specification.
     * @param guid[in] GuidPrefix_t to be included in the instance name.
     * @return The instance name.
     */
    std::string get_instance_name(
            const fastrtps::rtps::GuidPrefix_t guid) const;

    /**
     * Create the builtin endpoints used in the TypeLookupManager.
     * @return true if correct.
     */
    bool create_endpoints();

    //! Pointer to the local RTPSParticipant.
    fastrtps::rtps::RTPSParticipantImpl* participant_ = nullptr;

    //! Own instance name.
    std::string local_instance_name_;

    //! Pointer to the BuiltinProtocols class.
    fastrtps::rtps::BuiltinProtocols* builtin_protocols_ = nullptr;

    //! Pointer to the RTPSWriter for the TypeLookup_Request.
    fastrtps::rtps::StatefulWriter* builtin_request_writer_ = nullptr;

    //! Pointer to the RTPSReader for the TypeLookup_Request.
    fastrtps::rtps::StatefulReader* builtin_request_reader_ = nullptr;

    //! Pointer to the RTPSWriter for the TypeLookup_Reply.
    fastrtps::rtps::StatefulWriter* builtin_reply_writer_ = nullptr;

    //! Pointer to the RTPSReader for the TypeLookup_Reply.
    fastrtps::rtps::StatefulReader* builtin_reply_reader_ = nullptr;

    //! Pointer to the Writer History of TypeLookup_Request.
    fastrtps::rtps::WriterHistory* builtin_request_writer_history_ = nullptr;

    //! Pointer to the Writer History of TypeLookup_Reply.
    fastrtps::rtps::WriterHistory* builtin_reply_writer_history_ = nullptr;

    //! Pointer to the Reader History of TypeLookup_Request.
    fastrtps::rtps::ReaderHistory* builtin_request_reader_history_ = nullptr;

    //! Pointer to the Reader History of TypeLookup_Reply.
    fastrtps::rtps::ReaderHistory* builtin_reply_reader_history_ = nullptr;

    //! Request Listener object.
    TypeLookupRequestListener* request_listener_ = nullptr;

    //! Reply Listener object.
    TypeLookupReplyListener* reply_listener_ = nullptr;

    //! Mutex to protect access to temp_reader_proxy_data_ and temp_writer_proxy_data_.
    std::mutex temp_data_lock_;

    //! Pointer to the temp ReaderProxyData used for assigments.
    fastrtps::rtps::ReaderProxyData* temp_reader_proxy_data_ = nullptr;

    //! Pointer to the temp WriterProxyData used for assigments.
    fastrtps::rtps::WriterProxyData* temp_writer_proxy_data_ = nullptr;

    mutable fastrtps::rtps::SequenceNumber_t request_seq_number_;
    mutable TypeLookup_RequestPubSubType request_type_;
    mutable TypeLookup_ReplyPubSubType reply_type_;

    //! Mutex to protect access to async_get_type_callbacks_ and async_get_type_requests_.
    std::mutex async_get_types_mutex_;

    //! Collection of all the WriterProxyData and their callbacks related to a TypeIdentfierWithSize, hashed by its TypeIdentfierWithSize.
    std::unordered_map < xtypes::TypeIdentfierWithSize,
            std::vector<std::pair<eprosima::ProxyPool<eprosima::fastrtps::rtps::WriterProxyData>::smart_ptr,
            AsyncGetTypeWriterCallback>>> async_get_type_writer_callbacks_;

    //! Collection of all the ReaderProxyData and their callbacks related to a TypeIdentfierWithSize, hashed by its TypeIdentfierWithSize.
    std::unordered_map < xtypes::TypeIdentfierWithSize,
            std::vector<std::pair<eprosima::ProxyPool<eprosima::fastrtps::rtps::ReaderProxyData>::smart_ptr,
            AsyncGetTypeReaderCallback>>> async_get_type_reader_callbacks_;

    //! Collection SampleIdentity and the TypeIdentfierWithSize it originated from, hashed by its SampleIdentity.
    std::unordered_map<SampleIdentity, xtypes::TypeIdentfierWithSize> async_get_type_requests_;
};

} /* namespace builtin */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif /* _FASTDDS_TYPELOOKUP_SERVICE_MANAGER_HPP */

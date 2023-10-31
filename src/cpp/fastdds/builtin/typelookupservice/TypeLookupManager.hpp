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

#include <fastdds/builtin/typelookupservice/TypeLookupRequestListener.hpp>
#include <fastdds/builtin/typelookupservice/TypeLookupReplyListener.hpp>
#include <fastdds/builtin/typelookupservice/TypeLookupTypes.h>
#include <fastdds/builtin/typelookupservice/TypeLookupTypesPubSubTypes.h>
#include <fastdds/dds/xtypes/type_representation/TypeObjectRegistry.hpp>

#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/builtin/data/ReaderProxyData.h>


using GuidPrefix_t = eprosima::fastrtps::rtps::GuidPrefix_t;
using EntityId_t = eprosima::fastrtps::rtps::EntityId_t;
using GUID_t = eprosima::fastrtps::rtps::GUID_t;
using SequenceNumber_t = eprosima::fastrtps::rtps::SequenceNumber_t;
using SampleIdentity = eprosima::fastrtps::rtps::SampleIdentity;


// Changes get_types reply policy according to 7.6.3.3.4.2 of the XTypes 1.3 document.
// If true, mng will reply to get_types with EK_MINIMAL types, when possible.
const bool GET_TYPES_REPLY_WITH_MINIMAL = false;



namespace std {

template<>
struct hash<eprosima::fastdds::dds::xtypes1_3::TypeIdentifierSeq>
{
    std::size_t operator()(
            const eprosima::fastdds::dds::xtypes1_3::TypeIdentifierSeq& seq) const
    {
        std::size_t hash = 0;
        std::hash<eprosima::fastdds::dds::xtypes1_3::TypeIdentifier> id_hasher;
        //Implement a custom hash function for TypeIdentifierSeq
        for (const eprosima::fastdds::dds::xtypes1_3::TypeIdentifier& identifier : seq)
        {
            // Combine the hash values of individual elements using the custom hash function
            hash ^= id_hasher(identifier);
        }
        return hash;
        
        //return id_hasher(seq[0]) ^= id_hasher(seq[seq.size()-1]);
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

extern const eprosima::fastrtps::rtps::SampleIdentity INVALID_SAMPLE_IDENTITY;

class TypeLookupManager
{
    friend class TypeLookupRequestListener;
    friend class TypeLookupReplyListener;

public:

    //BuiltinProtocols::initBuiltinProtocols
    TypeLookupManager(
            fastrtps::rtps::BuiltinProtocols* bp);

    virtual ~TypeLookupManager();

    //BuiltinProtocols::initBuiltinProtocols
    bool init_typelookup_service(
            fastrtps::rtps::RTPSParticipantImpl* pi);

    //PDPSimple::notifyAboveRemoteEndpoints
    bool assign_remote_endpoints(
            const fastrtps::rtps::ParticipantProxyData& ppd);

    //PDP::remove_remote_participant
    void remove_remote_endpoints(
            fastrtps::rtps::ParticipantProxyData* ppd);

    //DomainParticipantImpl::get_type_dependencies
    eprosima::fastrtps::rtps::SampleIdentity get_type_dependencies(
            const xtypes1_3::TypeIdentifierSeq& in) const;

    //DomainParticipantImpl::get_types
    eprosima::fastrtps::rtps::SampleIdentity get_types(
            const xtypes1_3::TypeIdentifierSeq& in) const;

private:
    bool create_endpoints();

    bool send_request(
            TypeLookup_Request& req) const;

    bool send_reply(
            TypeLookup_Reply& rep) const;

    bool request_reception(
            fastrtps::rtps::CacheChange_t& change,
            TypeLookup_Request& req) const;

    bool reply_reception(
            fastrtps::rtps::CacheChange_t& change,
            TypeLookup_Reply& rep) const;

    fastrtps::types::ReturnCode_t get_registered_type_object(
            const TypeLookup_getTypes_In& in,
            TypeLookup_getTypes_Out& out);

    fastrtps::types::ReturnCode_t get_registered_type_dependencies(
        const TypeLookup_getTypeDependencies_In& in,
        TypeLookup_getTypeDependencies_Out& out);


    std::string get_instanceName() const;
    fastrtps::rtps::RTPSParticipantImpl* get_RTPS_participant();
    fastrtps::rtps::BuiltinProtocols* get_builtin_protocols();
    fastrtps::rtps::StatefulWriter* get_builtin_request_writer();
    fastrtps::rtps::StatefulWriter* get_builtin_reply_writer();
    fastrtps::rtps::StatefulReader* get_builtin_request_reader();
    fastrtps::rtps::StatefulReader* get_builtin_reply_reader();
    fastrtps::rtps::WriterHistory* get_builtin_request_writer_history();
    fastrtps::rtps::WriterHistory* get_builtin_reply_writer_history();
    fastrtps::rtps::ReaderHistory* get_builtin_request_reader_history();
    fastrtps::rtps::ReaderHistory* get_builtin_reply_reader_history();


    std::string instance_name_;//As defined in 7.6.3.3.4 of the XTypes 1.3 document
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

    std::mutex temp_proxy_data_lock_;
    fastrtps::rtps::ReaderProxyData temp_reader_proxy_data_;
    fastrtps::rtps::WriterProxyData temp_writer_proxy_data_;

    mutable fastrtps::rtps::SequenceNumber_t request_seq_number_;
    mutable TypeLookup_RequestPubSubType request_type_;
    mutable TypeLookup_ReplyPubSubType reply_type_;

    TypeLookupRequestListener* request_listener_;
    TypeLookupReplyListener* reply_listener_;

    std::mutex dependencies_requests_cache_mutex;
    std::unordered_map<xtypes1_3::TypeIdentifierSeq, std::unordered_set<xtypes1_3::TypeIdentfierWithSize>> dependencies_requests_cache_;
};

} /* namespace builtin */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_TYPELOOKUP_SERVICE_MANAGER_HPP */

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
 * @file TypeLookupManager.hpp
 *
 */

#ifndef FASTDDS_FASTDDS_BUILTIN_TYPE_LOOKUP_SERVICE__TYPELOOKUPMANAGER_HPP
#define FASTDDS_FASTDDS_BUILTIN_TYPE_LOOKUP_SERVICE__TYPELOOKUPMANAGER_HPP

#include <gmock/gmock.h>

#include <mutex>
#include <unordered_map>
#include <vector>

#include <fastdds/builtin/type_lookup_service/detail/TypeLookupTypes.hpp>
#include <fastdds/builtin/type_lookup_service/detail/TypeLookupTypesPubSubTypes.hpp>
#include <fastdds/builtin/type_lookup_service/TypeLookupReplyListener.hpp>
#include <fastdds/builtin/type_lookup_service/TypeLookupRequestListener.hpp>
#include <fastdds/xtypes/type_representation/TypeIdentifierWithSizeHashSpecialization.h>

#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>

#include <fastdds/utils/TypePropagation.hpp>
#include <utils/ProxyPool.hpp>

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

namespace fastdds {
namespace rtps {

class BuiltinProtocols;
class ReaderHistory;
class RTPSParticipantImpl;
class StatefulReader;
class StatefulWriter;
class ParticipantProxyData;
class WriterHistory;

} // namespace rtps

namespace dds {
namespace builtin {

const SampleIdentity INVALID_SAMPLE_IDENTITY;

using AsyncGetTypeWriterCallback = std::function<
    void (eprosima::ProxyPool<eprosima::fastdds::rtps::WriterProxyData>::smart_ptr&)>;
using AsyncGetTypeReaderCallback = std::function<
    void (eprosima::ProxyPool<eprosima::fastdds::rtps::ReaderProxyData>::smart_ptr&)>;

class TypeLookupManager
{
    friend class TypeLookupRequestListener;
    friend class TypeLookupReplyListener;

public:

    TypeLookupManager()
    {
    }

    virtual ~TypeLookupManager()
    {
    }

    // *INDENT-OFF*

    MOCK_CONST_METHOD2(get_type_dependencies, SampleIdentity(
                const xtypes::TypeIdentifierSeq&,
                const fastdds::rtps::GUID_t&));

    MOCK_CONST_METHOD3(get_type_dependencies, SampleIdentity(
                const xtypes::TypeIdentifierSeq&,
                const fastdds::rtps::GUID_t&,
                const std::vector<uint8_t>&));

    MOCK_CONST_METHOD2(get_types, SampleIdentity(
                const xtypes::TypeIdentifierSeq&,
                const fastdds::rtps::GUID_t&));

    MOCK_METHOD2(async_get_type, ReturnCode_t(
                eprosima::ProxyPool<eprosima::fastdds::rtps::WriterProxyData>::smart_ptr&,
                const AsyncGetTypeWriterCallback&));

    MOCK_METHOD2(async_get_type, ReturnCode_t(
                eprosima::ProxyPool<eprosima::fastdds::rtps::ReaderProxyData>::smart_ptr&,
                const AsyncGetTypeReaderCallback&));

    MOCK_METHOD2(notify_callbacks, void(
                ReturnCode_t,
                const xtypes::TypeIdentfierWithSize&));

    MOCK_METHOD2(add_async_get_type_request, bool(
                const SampleIdentity&,
                const xtypes::TypeIdentfierWithSize&));

    MOCK_METHOD1(remove_async_get_type_callback, bool(
                const xtypes::TypeIdentfierWithSize&));

    MOCK_METHOD1(remove_async_get_type_request, bool(
                SampleIdentity));

    MOCK_CONST_METHOD1(send, bool(TypeLookup_Reply&));

    MOCK_CONST_METHOD1(send, bool(TypeLookup_Request&));

    MOCK_CONST_METHOD2(receive, bool(
                fastdds::rtps::CacheChange_t&,
                TypeLookup_Reply&));

    MOCK_CONST_METHOD2(receive, bool(
                fastdds::rtps::CacheChange_t&,
                TypeLookup_Request&));

    MOCK_METHOD1(remove_remote_endpoints, void(
                fastdds::rtps::ParticipantProxyData*));

    MOCK_METHOD1(remove_builtin_request_writer_history_change, void(
                fastdds::rtps::CacheChange_t*));

    MOCK_METHOD1(remove_builtin_reply_writer_history_change, void(
                fastdds::rtps::CacheChange_t*));
    // *INDENT-ON*

    xtypes::TypeKind get_type_kind_to_propagate() const
    {
        return xtypes::EK_COMPLETE;
    }

    fastdds::rtps::RTPSParticipantImpl* participant_ = nullptr;

    std::unordered_map<SampleIdentity, xtypes::TypeIdentfierWithSize> async_get_type_requests_;

    mutable TypeLookup_ReplyPubSubType reply_type_;

};

} /* namespace builtin */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif // FASTDDS_FASTDDS_BUILTIN_TYPE_LOOKUP_SERVICE__TYPELOOKUPMANAGER_HPP

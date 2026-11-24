// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_TEST_MOCK_FASTDDS_BUILTIN_TYPE_LOOKUP_SERVICE__TYPELOOKUPMANAGERMOCK_HPP
#define FASTDDS_TEST_MOCK_FASTDDS_BUILTIN_TYPE_LOOKUP_SERVICE__TYPELOOKUPMANAGERMOCK_HPP

#include <cstdint>
#include <vector>

#include <gmock/gmock.h>

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>
#include <mock/MockCompanion.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace builtin {

/**
 * Expectations of a TypeLookupManager created with the mock implementation, reached with mock::mock_of().
 */
class TypeLookupManagerMock
{
public:

    MOCK_METHOD(void, remove_remote_endpoints, (fastdds::rtps::ParticipantProxyData*));

    MOCK_METHOD(SampleIdentity, get_type_dependencies,
            (const xtypes::TypeIdentifierSeq&, const fastdds::rtps::GUID_t&, const std::vector<uint8_t>&));

    MOCK_METHOD(SampleIdentity, get_types, (const xtypes::TypeIdentifierSeq&, const fastdds::rtps::GUID_t&));

    MOCK_METHOD(ReturnCode_t, async_get_type,
            (eprosima::ProxyPool<fastdds::rtps::WriterProxyData>::smart_ptr&, const fastdds::rtps::GUID_t&,
            const AsyncGetTypeWriterCallback&));

    MOCK_METHOD(ReturnCode_t, async_get_type,
            (eprosima::ProxyPool<fastdds::rtps::ReaderProxyData>::smart_ptr&, const fastdds::rtps::GUID_t&,
            const AsyncGetTypeReaderCallback&));

    MOCK_METHOD(void, notify_callbacks, (ReturnCode_t, const xtypes::TypeIdentfierWithSize&));

    MOCK_METHOD(bool, add_async_get_type_request,
            (const SampleIdentity&, const xtypes::TypeIdentfierWithSize&, const fastdds::rtps::GUID_t&));

    MOCK_METHOD(bool, remove_async_get_type_request, (SampleIdentity));

    MOCK_METHOD(bool, send, (TypeLookup_Request &));

    MOCK_METHOD(bool, send, (TypeLookup_Reply &));

    MOCK_METHOD(bool, receive, (fastdds::rtps::CacheChange_t&, TypeLookup_Request &));

    MOCK_METHOD(bool, receive, (fastdds::rtps::CacheChange_t&, TypeLookup_Reply &));

    MOCK_METHOD(void, remove_builtin_request_writer_history_change, (fastdds::rtps::CacheChange_t*));

    MOCK_METHOD(void, remove_builtin_reply_writer_history_change, (fastdds::rtps::CacheChange_t*));
};

} // namespace builtin
} // namespace dds

namespace mock {

template<>
struct MockOf<dds::builtin::TypeLookupManager>
{
    using type = dds::builtin::TypeLookupManagerMock;
};

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_FASTDDS_BUILTIN_TYPE_LOOKUP_SERVICE__TYPELOOKUPMANAGERMOCK_HPP

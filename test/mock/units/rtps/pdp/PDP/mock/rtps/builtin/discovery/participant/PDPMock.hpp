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

#ifndef FASTDDS_TEST_MOCK_RTPS_BUILTIN_DISCOVERY_PARTICIPANT__PDPMOCK_HPP
#define FASTDDS_TEST_MOCK_RTPS_BUILTIN_DISCOVERY_PARTICIPANT__PDPMOCK_HPP

#include <gmock/gmock.h>

#include <fastdds/rtps/attributes/RTPSParticipantAllocationAttributes.hpp>
#include <mock/MockCompanion.hpp>
#include <rtps/builtin/discovery/participant/PDP.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Expectations on the non-virtual methods of a PDP created with the mock implementation, reached with
 * mock::mock_of().
 */
class PDPMock
{
public:

    MOCK_METHOD(ReaderProxyData*, addReaderProxyData,
            (const GUID_t&, GUID_t&, std::function<bool(ReaderProxyData*, bool, const ParticipantProxyData&)>));

    MOCK_METHOD(WriterProxyData*, addWriterProxyData,
            (const GUID_t&, GUID_t&, std::function<bool(WriterProxyData*, bool, const ParticipantProxyData&)>));

    MOCK_METHOD(CDRMessage_t, get_participant_proxy_data_serialized, (Endianness_t));

    MOCK_METHOD(void, notify_incompatible_qos_matching,
            (const GUID_t&, const GUID_t&, const fastdds::dds::PolicyMask&));
};

/**
 * Concrete PDP for tests, with its pure virtual methods mocked.
 */
class TestPDP : public PDP
{
public:

    TestPDP()
        : PDP(nullptr, c_default_RTPSParticipantAllocationAttributes)
    {
    }

    using PDP::announceParticipantState;

    MOCK_METHOD(bool, init, (RTPSParticipantImpl*), (override));

    MOCK_METHOD(ParticipantProxyData*, createParticipantProxyData, (const ParticipantProxyData&, const GUID_t&),
            (override));

    MOCK_METHOD(void, announceParticipantState, (bool, bool, WriteParams &), (override));

    MOCK_METHOD(bool, createPDPEndpoints, (), (override));

    MOCK_METHOD(void, assignRemoteEndpoints, (ParticipantProxyData*), (override));

    MOCK_METHOD(void, notifyAboveRemoteEndpoints, (const ParticipantProxyData&, bool), (override));

    MOCK_METHOD(void, removeRemoteEndpoints, (ParticipantProxyData*), (override));

    MOCK_METHOD(void, update_builtin_locators, (), (override));
};

} // namespace rtps

namespace mock {

template<>
struct MockOf<rtps::PDP>
{
    using type = testing::NiceMock<rtps::PDPMock>;
};

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_RTPS_BUILTIN_DISCOVERY_PARTICIPANT__PDPMOCK_HPP

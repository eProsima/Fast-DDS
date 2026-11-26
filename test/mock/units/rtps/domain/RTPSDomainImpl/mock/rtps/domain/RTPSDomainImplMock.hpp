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

#ifndef FASTDDS_TEST_MOCK_RTPS_DOMAIN__RTPSDOMAINIMPLMOCK_HPP
#define FASTDDS_TEST_MOCK_RTPS_DOMAIN__RTPSDOMAINIMPLMOCK_HPP

#include <cstdint>
#include <memory>
#include <string>

#include <gmock/gmock.h>

#include <mock/MockCompanion.hpp>
#include <rtps/domain/RTPSDomainImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Expectations of a RTPSDomainImpl created with the mock implementation, reached with mock::mock_of().
 * It is a NiceMock: methods without expectations return default values silently, as no test needs the real domain.
 */
class RTPSDomainImplMock
{
public:

    MOCK_METHOD(void, stop_all, ());

    MOCK_METHOD(RTPSParticipant*, create_participant,
            (uint32_t, bool, const RTPSParticipantAttributes&, RTPSParticipantListener*));

    MOCK_METHOD(RTPSParticipant*, create_client_server_participant,
            (uint32_t, bool, const RTPSParticipantAttributes&, RTPSParticipantListener*));

    MOCK_METHOD(bool, remove_writer, (RTPSWriter*));

    MOCK_METHOD(bool, remove_reader, (RTPSReader*));

    MOCK_METHOD(bool, remove_participant, (RTPSParticipant*));

    MOCK_METHOD(RTPSWriter*, create_writer,
            (RTPSParticipant*, const EntityId_t&, WriterAttributes&, WriterHistory*, WriterListener*));

    MOCK_METHOD(RTPSParticipantImpl*, find_participant, (const GUID_t&));

    MOCK_METHOD(void, find_reader, (std::shared_ptr<LocalReaderPointer>&, const GUID_t&));

    MOCK_METHOD(BaseWriter*, find_writer, (const GUID_t&));

    MOCK_METHOD(void, file_watch_callback, ());

    MOCK_METHOD(void, set_filewatch_thread_config, (const ThreadSettings&, const ThreadSettings&));

    MOCK_METHOD(bool, run_easy_mode_discovery_server, (uint32_t, const std::string&));
};

} // namespace rtps

namespace mock {

template<>
struct MockOf<rtps::RTPSDomainImpl>
{
    using type = testing::NiceMock<rtps::RTPSDomainImplMock>;
};

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_RTPS_DOMAIN__RTPSDOMAINIMPLMOCK_HPP

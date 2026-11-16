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

#ifndef FASTDDS_TEST_MOCK_FASTDDS_RTPS__RTPSDOMAINMOCK_HPP
#define FASTDDS_TEST_MOCK_FASTDDS_RTPS__RTPSDOMAINMOCK_HPP

#include <cstdint>
#include <memory>

#include <gmock/gmock.h>

#include <fastdds/rtps/RTPSDomain.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Expectations of the RTPSDomain mock implementation.
 *
 * While an instance exists, the static functions of RTPSDomain forward to it. Otherwise they do nothing and return
 * nullptr or false. Only one instance can exist at a time.
 */
class RTPSDomainMock
{
public:

    RTPSDomainMock()
    {
        instance() = this;
    }

    virtual ~RTPSDomainMock()
    {
        instance() = nullptr;
    }

    MOCK_METHOD(void, set_filewatch_thread_config, (const ThreadSettings&, const ThreadSettings&));

    MOCK_METHOD(void, stopAll, ());

    /**
     * Called by RTPSDomain::createParticipant() with the participant it created, which has a RTPSParticipantMock
     * companion. Returning false destroys it, and RTPSDomain::createParticipant() returns nullptr.
     */
    MOCK_METHOD(bool, createParticipant,
            (RTPSParticipant*, uint32_t, bool, const RTPSParticipantAttributes&, RTPSParticipantListener*));

    /**
     * Called by RTPSDomain::create_client_server_participant() with the participant it created, which has a
     * RTPSParticipantMock companion. Returning false destroys it, and the function returns nullptr.
     */
    MOCK_METHOD(bool, create_client_server_participant,
            (RTPSParticipant*, uint32_t, bool, const RTPSParticipantAttributes&, RTPSParticipantListener*));

    MOCK_METHOD(RTPSWriter*, createRTPSWriter,
            (RTPSParticipant*, WriterAttributes&, WriterHistory*, WriterListener*));

    MOCK_METHOD(RTPSWriter*, createRTPSWriter,
            (RTPSParticipant*, const EntityId_t&, WriterAttributes&, WriterHistory*, WriterListener*));

    MOCK_METHOD(bool, removeRTPSWriter, (RTPSWriter*));

    MOCK_METHOD(RTPSReader*, createRTPSReader,
            (RTPSParticipant*, ReaderAttributes&, ReaderHistory*, ReaderListener*));

    MOCK_METHOD(RTPSReader*, createRTPSReader,
            (RTPSParticipant*, ReaderAttributes&, const std::shared_ptr<IPayloadPool>&, ReaderHistory*,
            ReaderListener*));

    MOCK_METHOD(RTPSReader*, createRTPSReader,
            (RTPSParticipant*, const EntityId_t&, ReaderAttributes&, const std::shared_ptr<IPayloadPool>&,
            ReaderHistory*, ReaderListener*));

    MOCK_METHOD(bool, removeRTPSReader, (RTPSReader*));

    //! Returning true makes RTPSDomain::removeRTPSParticipant() destroy the participant.
    MOCK_METHOD(bool, removeRTPSParticipant, (RTPSParticipant*));

    MOCK_METHOD(bool, get_library_settings, (LibrarySettings &));

    MOCK_METHOD(bool, set_library_settings, (const LibrarySettings&));

    /**
     * Get the instance the RTPSDomain mock implementation forwards to.
     *
     * @return Reference to the pointer to the existing instance, nullptr if there is none.
     */
    static RTPSDomainMock*& instance();
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_FASTDDS_RTPS__RTPSDOMAINMOCK_HPP

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

// Link-seam implementation of RTPSDomain: replaces the RTPSDomain functions of src/cpp/rtps/domain/RTPSDomain.cpp and
// forwards them to the RTPSDomainMock instance, if any. Otherwise they do nothing and return nullptr or false.
// RTPSParticipant can only be created and destroyed by its friends, so participants are created and destroyed here,
// with the RTPSParticipant mock implementation.

#include <fastdds/rtps/RTPSDomain.hpp>

#include <fastdds/rtps/participant/RTPSParticipant.hpp>

#include <mock/fastdds/rtps/RTPSDomainMock.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

RTPSDomainMock*& RTPSDomainMock::instance()
{
    static RTPSDomainMock* instance = nullptr;
    return instance;
}

void RTPSDomain::set_filewatch_thread_config(
        const ThreadSettings& watch_thread,
        const ThreadSettings& callback_thread)
{
    if (nullptr != RTPSDomainMock::instance())
    {
        RTPSDomainMock::instance()->set_filewatch_thread_config(watch_thread, callback_thread);
    }
}

void RTPSDomain::stopAll()
{
    if (nullptr != RTPSDomainMock::instance())
    {
        RTPSDomainMock::instance()->stopAll();
    }
}

RTPSParticipant* RTPSDomain::createParticipant(
        uint32_t domain_id,
        const RTPSParticipantAttributes& attrs,
        RTPSParticipantListener* plisten)
{
    return createParticipant(domain_id, true, attrs, plisten);
}

RTPSParticipant* RTPSDomain::createParticipant(
        uint32_t domain_id,
        bool enabled,
        const RTPSParticipantAttributes& attrs,
        RTPSParticipantListener* plisten)
{
    if (nullptr != RTPSDomainMock::instance())
    {
        RTPSParticipant* participant = new RTPSParticipant(nullptr);
        if (RTPSDomainMock::instance()->createParticipant(participant, domain_id, enabled, attrs, plisten))
        {
            return participant;
        }
        delete participant;
    }
    return nullptr;
}

RTPSParticipant* RTPSDomain::create_client_server_participant(
        uint32_t domain_id,
        bool enabled,
        const RTPSParticipantAttributes& attrs,
        RTPSParticipantListener* plisten)
{
    if (nullptr != RTPSDomainMock::instance())
    {
        RTPSParticipant* participant = new RTPSParticipant(nullptr);
        if (RTPSDomainMock::instance()->create_client_server_participant(participant, domain_id, enabled, attrs,
                plisten))
        {
            return participant;
        }
        delete participant;
    }
    return nullptr;
}

RTPSWriter* RTPSDomain::createRTPSWriter(
        RTPSParticipant* p,
        WriterAttributes& watt,
        WriterHistory* hist,
        WriterListener* listen)
{
    if (nullptr != RTPSDomainMock::instance())
    {
        return RTPSDomainMock::instance()->createRTPSWriter(p, watt, hist, listen);
    }
    return nullptr;
}

RTPSWriter* RTPSDomain::createRTPSWriter(
        RTPSParticipant* p,
        const EntityId_t& entity_id,
        WriterAttributes& watt,
        WriterHistory* hist,
        WriterListener* listen)
{
    if (nullptr != RTPSDomainMock::instance())
    {
        return RTPSDomainMock::instance()->createRTPSWriter(p, entity_id, watt, hist, listen);
    }
    return nullptr;
}

bool RTPSDomain::removeRTPSWriter(
        RTPSWriter* writer)
{
    if (nullptr != RTPSDomainMock::instance())
    {
        return RTPSDomainMock::instance()->removeRTPSWriter(writer);
    }
    return false;
}

RTPSReader* RTPSDomain::createRTPSReader(
        RTPSParticipant* p,
        ReaderAttributes& ratt,
        ReaderHistory* hist,
        ReaderListener* listen)
{
    if (nullptr != RTPSDomainMock::instance())
    {
        return RTPSDomainMock::instance()->createRTPSReader(p, ratt, hist, listen);
    }
    return nullptr;
}

RTPSReader* RTPSDomain::createRTPSReader(
        RTPSParticipant* p,
        ReaderAttributes& ratt,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        ReaderHistory* hist,
        ReaderListener* listen)
{
    if (nullptr != RTPSDomainMock::instance())
    {
        return RTPSDomainMock::instance()->createRTPSReader(p, ratt, payload_pool, hist, listen);
    }
    return nullptr;
}

RTPSReader* RTPSDomain::createRTPSReader(
        RTPSParticipant* p,
        const EntityId_t& entity_id,
        ReaderAttributes& ratt,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        ReaderHistory* hist,
        ReaderListener* listen)
{
    if (nullptr != RTPSDomainMock::instance())
    {
        return RTPSDomainMock::instance()->createRTPSReader(p, entity_id, ratt, payload_pool, hist, listen);
    }
    return nullptr;
}

bool RTPSDomain::removeRTPSReader(
        RTPSReader* reader)
{
    if (nullptr != RTPSDomainMock::instance())
    {
        return RTPSDomainMock::instance()->removeRTPSReader(reader);
    }
    return false;
}

bool RTPSDomain::removeRTPSParticipant(
        RTPSParticipant* p)
{
    if (nullptr != RTPSDomainMock::instance() && RTPSDomainMock::instance()->removeRTPSParticipant(p))
    {
        delete p;
        return true;
    }
    return false;
}

bool RTPSDomain::get_library_settings(
        LibrarySettings& library_settings)
{
    if (nullptr != RTPSDomainMock::instance())
    {
        return RTPSDomainMock::instance()->get_library_settings(library_settings);
    }
    return false;
}

bool RTPSDomain::set_library_settings(
        const LibrarySettings& library_settings)
{
    if (nullptr != RTPSDomainMock::instance())
    {
        return RTPSDomainMock::instance()->set_library_settings(library_settings);
    }
    return false;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

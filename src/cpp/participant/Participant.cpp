// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Participant.cpp
 *
 */

#include <fastrtps/participant/Participant.h>

#include "ParticipantImpl.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

Participant::Participant():
    mp_impl(nullptr)
    {

    }

Participant::~Participant() {
    // TODO Auto-generated destructor stub
}

const GUID_t& Participant::getGuid() const
{
    return mp_impl->getGuid();
}

const ParticipantAttributes& Participant::getAttributes() const
{
    return mp_impl->getAttributes();
}

bool Participant::newRemoteEndpointDiscovered(const GUID_t& partguid, uint16_t endpointId,
        EndpointKind_t kind)
{
    return mp_impl->newRemoteEndpointDiscovered(partguid, endpointId, kind);
}

std::vector<std::string> Participant::getParticipantNames() const {
    return mp_impl->getParticipantNames();
}

bool Participant::get_remote_writer_info(const GUID_t& writerGuid, WriterProxyData& returnedInfo)
{
    return mp_impl->get_remote_writer_info(writerGuid, returnedInfo);
}

bool Participant::get_remote_reader_info(const GUID_t& readerGuid, ReaderProxyData& returnedInfo)
{
    return mp_impl->get_remote_reader_info(readerGuid, returnedInfo);
}

ResourceEvent& Participant::get_resource_event() const
{
    return mp_impl->get_resource_event();
}

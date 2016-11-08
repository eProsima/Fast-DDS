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
 * @file RTPSParticipantImpl.h
 */

#ifndef RTPS_PARTICIPANT_RTPSPARTICIPANTIMPL_H_
#define RTPS_PARTICIPANT_RTPSPARTICIPANTIMPL_H_

#include <fastrtps/rtps/attributes/WriterAttributes.h>
#include <fastrtps/rtps/attributes/ReaderAttributes.h>

#include <gmock/gmock.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSWriter;
class RTPSReader;
class WriterHistory;
class ReaderHistory;
class WriterListener;
class ReaderListener;
class EntityId_t;

class RTPSParticipantImpl
{
    public:

        MOCK_CONST_METHOD0(getRTPSParticipantAttributes, const RTPSParticipantAttributes&());

        MOCK_CONST_METHOD0(getGuid, const GUID_t&());

        MOCK_METHOD6(createWriter, bool (RTPSWriter** Writer, WriterAttributes& param, WriterHistory* hist,WriterListener* listen,
                const EntityId_t& entityId, bool isBuiltin));

        MOCK_METHOD7(createReader, bool (RTPSReader** Reader, ReaderAttributes& param, ReaderHistory* hist,ReaderListener* listen,
                const EntityId_t& entityId, bool isBuiltin, bool enable));
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // RTPS_PARTICIPANT_RTPSPARTICIPANTIMPL_H_


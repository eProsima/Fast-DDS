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

#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastrtps/rtps/attributes/ReaderAttributes.h>
#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include <fastrtps/rtps/participant/RTPSParticipantListener.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>

#include <gmock/gmock.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class Endpoint;
class RTPSParticipant;
class WriterHistory;
class ReaderHistory;
class WriterListener;
class ReaderListener;
struct EntityId_t;

class MockParticipantListener : public RTPSParticipantListener
{
    public:

        MOCK_METHOD2(onRTPSParticipantDiscovery, void (RTPSParticipant*, RTPSParticipantDiscoveryInfo));

        MOCK_METHOD2(onRTPSParticipantAuthentication, void (RTPSParticipant*, const RTPSParticipantAuthenticationInfo&));
};

class RTPSParticipantImpl
{
    public:

        RTPSParticipantImpl()
        {
            events_.init_thread(this);
        }

        MOCK_CONST_METHOD0(getRTPSParticipantAttributes, const RTPSParticipantAttributes&());

        MOCK_CONST_METHOD0(getGuid, const GUID_t&());

        MOCK_METHOD1(setGuid, void(GUID_t&));

        MOCK_METHOD6(createWriter_mock, bool (RTPSWriter** writer, WriterAttributes& param, WriterHistory* hist,WriterListener* listen,
                const EntityId_t& entityId, bool isBuiltin));

        MOCK_METHOD7(createReader_mock, bool (RTPSReader** reader, ReaderAttributes& param, ReaderHistory* hist,ReaderListener* listen,
                const EntityId_t& entityId, bool isBuiltin, bool enable));

        bool createWriter(RTPSWriter** writer, WriterAttributes& param, WriterHistory* hist, WriterListener* listen,
                const EntityId_t& entityId, bool isBuiltin)
        {
            bool ret = createWriter_mock(writer, param , hist, listen, entityId, isBuiltin);
            if(*writer != nullptr)
                (*writer)->history_ = hist;
            return ret;
        }

        bool createReader(RTPSReader** reader, ReaderAttributes& param, ReaderHistory* hist,ReaderListener* listen,
                const EntityId_t& entityId, bool isBuiltin, bool enable)
        {
            bool ret = createReader_mock(reader, param, hist, listen, entityId, isBuiltin, enable);
            if(*reader != nullptr)
            {
                (*reader)->history_ = hist;
                (*reader)->listener_ = listen;
            }
            return ret;
        }

        void deleteUserEndpoint(Endpoint* endpoint) { delete endpoint; }

        PDPSimple* pdpsimple() { return &pdpsimple_; }

        MockParticipantListener* getListener() { return &listener_; }

        RTPSParticipant* getUserRTPSParticipant() { return nullptr; }

        ResourceEvent& getEventResource() { return events_; }

        void set_endpoint_rtps_protection_supports(Endpoint* /*endpoint*/, bool /*support*/) {}

        void ResourceSemaphoreWait() {}
        void ResourceSemaphorePost() {}

        uint32_t getMaxMessageSize() const { return 65536; }

    private:

        PDPSimple pdpsimple_;

        MockParticipantListener listener_;

        ResourceEvent events_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // RTPS_PARTICIPANT_RTPSPARTICIPANTIMPL_H_


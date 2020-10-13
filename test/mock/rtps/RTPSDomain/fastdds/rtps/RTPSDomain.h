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

#ifndef _FASTDDS_RTPS_DOMAIN_H_
#define _FASTDDS_RTPS_DOMAIN_H_

#include <fastdds/rtps/common/Types.h>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>

#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/writer/RTPSWriter.h>

#include <gmock/gmock.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSParticipant;
class RTPSParticipantListener;
class RTPSWriter;
class WriterAttributes;
class WriterHistory;
class WriterListener;
class RTPSReader;
class ReaderAttributes;
class ReaderHistory;
class ReaderListener;
class RTPSDomainImpl;

class RTPSDomain
{
public:

    static void stopAll()
    {
    }

    static RTPSParticipant* createParticipant(
            uint32_t,
            const RTPSParticipantAttributes&,
            RTPSParticipantListener* plisten = nullptr)
    {
        participant_->set_listener(plisten);
        return participant_;
    }

    static RTPSParticipant* createParticipant(
            uint32_t,
            bool,
            const RTPSParticipantAttributes&,
            RTPSParticipantListener* plisten = nullptr)
    {
        participant_->set_listener(plisten);
        return participant_;
    }

    static RTPSWriter* createRTPSWriter(
            RTPSParticipant*,
            WriterAttributes&,
            WriterHistory*,
            WriterListener* listen = nullptr)
    {
        writer_->set_listener(listen);
        return writer_;
    }

    static bool removeRTPSWriter(
            RTPSWriter*)
    {
        return true;
    }

    static RTPSReader* createRTPSReader(
            RTPSParticipant*,
            ReaderAttributes&,
            ReaderHistory*,
            ReaderListener* listen = nullptr)
    {
        reader_->setListener(listen);
        return reader_;
    }

    static RTPSReader* createRTPSReader(
            RTPSParticipant*,
            ReaderAttributes&,
            const std::shared_ptr<IPayloadPool>&,
            ReaderHistory*,
            ReaderListener* listen = nullptr)
    {
        reader_->setListener(listen);
        return reader_;
    }

    static bool removeRTPSReader(
            RTPSReader*)
    {
        return true;
    }

    static bool removeRTPSParticipant(
            RTPSParticipant*)
    {
        return true;
    }

    static inline void setMaxRTPSParticipantId(
            uint32_t maxRTPSParticipantId)
    {
        m_maxRTPSParticipantID = maxRTPSParticipantId;
    }

    static RTPSParticipant* clientServerEnvironmentCreationOverride(
            uint32_t,
            bool,
            const RTPSParticipantAttributes&,
            RTPSParticipantListener* listen /*= nullptr*/)
    {
        participant_->set_listener(listen);
        return participant_;
    }

    static inline uint32_t getNewId()
    {
        return m_maxRTPSParticipantID++;
    }

    static void removeRTPSParticipant_nts(
            std::pair<RTPSParticipant*, RTPSParticipantImpl*>&)
    {
    }

    RTPSDomain() = delete;

    ~RTPSDomain() = delete;

    static std::atomic<uint32_t> m_maxRTPSParticipantID;

    static RTPSReader* reader_;
    static RTPSWriter* writer_;
    static RTPSParticipant* participant_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif /* _FASTDDS_RTPS_DOMAIN_H_*/

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

#ifndef _RTPS_READER_STATEFULREADER_H_
#define _RTPS_READER_STATEFULREADER_H_

#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/attributes/ReaderAttributes.h>
#include <fastrtps/rtps/common/Guid.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSMessageGroup_t;
class WriterProxy;
class RTPSMessageSenderInterface;

class StatefulReader : public RTPSReader
{
    public:

        virtual ~StatefulReader() {}

        MOCK_METHOD1(matched_writer_add, bool(const WriterProxyData&));

        MOCK_METHOD1(matched_writer_remove, bool(const GUID_t&));

        // In real class, inherited from Endpoint base class.
        inline const GUID_t& getGuid() const { return guid_; };

        inline ReaderTimes& getTimes(){return times_;};

        void send_acknack(
                const SequenceNumberSet_t& /*sns*/,
                RTPSMessageGroup_t& /*buffer*/,
                const RTPSMessageSenderInterface& /*sender*/,
                bool /*is_final*/)
        {}

        void send_acknack(
                const WriterProxy* /*writer*/,
                RTPSMessageGroup_t& /*buffer*/,
                const RTPSMessageSenderInterface& /*sender*/,
                bool /*heartbeat_was_final*/)
        {}

        RTPSParticipantImpl* getRTPSParticipant() const { return nullptr; }

        bool send_sync_nts(
                CDRMessage_t* /*message*/,
                const Locator_t& /*locator*/,
                std::chrono::steady_clock::time_point& /*max_blocking_time_point*/)
        {
            return true;
        }

    private:

        GUID_t guid_;

        ReaderTimes times_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _RTPS_READER_STATEFULREADER_H_

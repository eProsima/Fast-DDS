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
 * @file StatefulReader.hpp
 */

#ifndef RTPS_READER__STATEFULREADER_HPP
#define RTPS_READER__STATEFULREADER_HPP

#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastdds/rtps/common/Guid.h>
#include <rtps/reader/BaseReader.hpp>
#include <rtps/resources/ResourceEvent.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSMessageGroup_t;
class WriterProxy;
class RTPSMessageSenderInterface;
class RTPSParticipantImpl;
struct CDRMessage_t;

class StatefulReader : public fastdds::rtps::BaseReader
{
public:

    StatefulReader()
    {
        ON_CALL(*this, getEventResource())
                .WillByDefault(::testing::ReturnRef(service_));
    }

    StatefulReader(
            ReaderHistory* history,
            RecursiveTimedMutex* mutex)
        : fastdds::rtps::BaseReader(history, mutex)
    {
    }

    virtual ~StatefulReader()
    {
    }

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD1(matched_writer_add, bool(const WriterProxyData&));

    MOCK_METHOD2(matched_writer_remove, bool(const GUID_t&, bool));

    MOCK_METHOD1(liveliness_expired, bool(const GUID_t&));

    MOCK_METHOD2(change_received, bool(CacheChange_t* a_change, WriterProxy* prox));

    MOCK_METHOD1 (matched_writer_is_matched, bool(const GUID_t& writer_guid));

    MOCK_METHOD1 (assert_writer_liveliness, void(const GUID_t& writer_guid));

    MOCK_METHOD0 (is_in_clean_state, bool());
    // *INDENT-ON*

    ReaderTimes& getTimes()
    {
        return times_;
    }

    void send_acknack(
            const WriterProxy* /*writer*/,
            const SequenceNumberSet_t& sns,
            const RTPSMessageSenderInterface* /*sender*/,
            bool /*is_final*/)
    {
        // only insterested in SequenceNumberSet_t.
        simp_send_acknack( sns );
    }

    // See gmock cookbook #SimplerInterfaces
    MOCK_METHOD1(simp_send_acknack, void(const SequenceNumberSet_t& seq));

    void send_acknack(
            const WriterProxy* /*writer*/,
            const RTPSMessageSenderInterface* /*sender*/,
            bool /*heartbeat_was_final*/)
    {
    }

    RTPSParticipantImpl* getRTPSParticipant() const
    {
        return nullptr;
    }

    MOCK_METHOD0(getEventResource, ResourceEvent & ());

    bool send_sync_nts(
            CDRMessage_t* /*message*/,
            const LocatorsIterator& /*destination_locators_begin*/,
            const LocatorsIterator& /*destination_locators_end*/,
            std::chrono::steady_clock::time_point& /*max_blocking_time_point*/)
    {
        return true;
    }

private:

    ReaderTimes times_;
    ResourceEvent service_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // RTPS_READER__STATEFULREADER_HPP

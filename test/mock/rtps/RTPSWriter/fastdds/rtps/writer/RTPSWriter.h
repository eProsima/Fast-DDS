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
 * @file RTPSWriter.h
 */

#ifndef _FASTDDS_RTPS_RTPSWRITER_H_
#define _FASTDDS_RTPS_RTPSWRITER_H_

#include <fastrtps/rtps/attributes/WriterAttributes.h>
#include <fastrtps/rtps/writer/WriterListener.h>
#include <fastrtps/rtps/Endpoint.h>
#include <fastrtps/rtps/common/CacheChange.h>
#include <fastdds/rtps/messages/RTPSMessageGroup.h>

#include <condition_variable>
#include <gmock/gmock.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class WriterHistory;
class RTPSParticipantImpl;
class FlowController;

class RTPSWriter : public Endpoint
{
public:

    virtual ~RTPSWriter() = default;

    virtual bool matched_reader_add(
            const ReaderProxyData& ratt) = 0;

    virtual bool matched_reader_remove(
            const GUID_t& ratt) = 0;

    virtual bool matched_reader_is_matched(
            const GUID_t& rguid) = 0;

    WriterListener* getListener() const
    {
        return listener_;
    }

    bool set_listener(
            WriterListener* listener)
    {
        listener_ = listener;
        return true;
    }

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_CONST_METHOD0(getGuid, const GUID_t& ());

    MOCK_METHOD3(new_change, CacheChange_t* (
            const std::function<uint32_t()>&,
            ChangeKind_t,
            InstanceHandle_t));

    MOCK_METHOD2(new_change, CacheChange_t* (
            const std::function<uint32_t()>&,
            ChangeKind_t));

    MOCK_METHOD1(set_separate_sending, void(bool));

    MOCK_METHOD0(getRTPSParticipant, RTPSParticipantImpl* ());

    MOCK_METHOD0 (getTypeMaxSerialized, uint32_t());

    MOCK_METHOD1(calculateMaxDataSize, uint32_t(uint32_t));

    MOCK_METHOD0(getMaxDataSize, uint32_t ());

    MOCK_CONST_METHOD0(get_liveliness_kind, const LivelinessQosPolicyKind& ());

    MOCK_CONST_METHOD0(get_liveliness_lease_duration, const Duration_t& ());
    // *INDENT-ON*

    virtual void updateAttributes(
            const WriterAttributes&)
    {
    }

    virtual void send_any_unsent_changes()
    {
    }

    virtual bool try_remove_change(
            const std::chrono::steady_clock::time_point&,
            std::unique_lock<RecursiveTimedMutex>&)
    {
        return true;
    }

    virtual void add_flow_controller(
            std::unique_ptr<FlowController>)
    {
    }

    virtual void unsent_change_added_to_history(
            CacheChange_t*,
            const std::chrono::time_point<std::chrono::steady_clock>&)
    {
    }

    virtual bool change_removed_by_history(
            CacheChange_t*)
    {
        return true;
    }

    virtual bool is_acked_by_all(
            const CacheChange_t* /*a_change*/) const
    {
        return false;
    }

    virtual bool wait_for_all_acked(
            const Duration_t& /*max_wait*/)
    {
        return true;
    }

    virtual bool process_acknack(
            const GUID_t& writer_guid,
            const GUID_t& reader_guid,
            uint32_t ack_count,
            const SequenceNumberSet_t& sn_set,
            bool final_flag,
            bool& result)
    {
        (void)writer_guid; (void)reader_guid; (void)ack_count; (void)sn_set; (void)final_flag;

        result = false;
        return true;
    }

    virtual bool process_nack_frag(
            const GUID_t& writer_guid,
            const GUID_t& reader_guid,
            uint32_t ack_count,
            const SequenceNumber_t& seq_num,
            const FragmentNumberSet_t fragments_state,
            bool& result)
    {
        (void)reader_guid; (void)ack_count; (void)seq_num; (void)fragments_state;

        result = false;
        return writer_guid == m_guid;
    }

    WriterHistory* history_;

    WriterListener* listener_;

    const GUID_t m_guid;

    LivelinessLostStatus liveliness_lost_status_;

};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_RTPS_RTPSWRITER_H_

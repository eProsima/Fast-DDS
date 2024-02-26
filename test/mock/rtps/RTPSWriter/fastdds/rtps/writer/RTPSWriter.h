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

#include <condition_variable>

#include <gmock/gmock.h>

#include <fastdds/rtps/common/VendorId_t.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/messages/RTPSMessageGroup.h>
#include <fastdds/rtps/writer/DeliveryRetCode.hpp>
#include <fastdds/rtps/writer/LocatorSelectorSender.hpp>
#include <fastrtps/rtps/attributes/WriterAttributes.h>
#include <fastrtps/rtps/common/CacheChange.h>
#include <fastrtps/rtps/Endpoint.h>
#include <fastrtps/rtps/writer/WriterListener.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class WriterHistory;
class RTPSParticipantImpl;

class RTPSWriter : public Endpoint
{
public:

    RTPSWriter()
        : general_locator_selector_(*this, ResourceLimitedContainerConfig())
        , async_locator_selector_(*this, ResourceLimitedContainerConfig())
    {
        static uint8_t entity_id = 0;
        // Generate a guid.
        m_guid.entityId.value[3] = ++entity_id;
    }

    virtual ~RTPSWriter() = default;

    virtual bool matched_reader_add(
            const ReaderProxyData&)
    {
        return false;
    }

    virtual bool matched_reader_remove(
            const GUID_t&)
    {
        return false;
    }

    virtual bool matched_reader_is_matched(
            const GUID_t&)
    {
        return false;
    }

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

    virtual bool get_disable_positive_acks() const
    {
        return false;
    }

    virtual bool has_been_fully_delivered(
            const SequenceNumber_t& /*seq_num*/) const
    {
        return false;
    }

#ifdef FASTDDS_STATISTICS

    template<typename T>
    bool add_statistics_listener(
            T /*listener*/)
    {
        return true;
    }

    template<typename T>
    bool remove_statistics_listener(
            T /*listener*/)
    {
        return true;
    }

#endif // FASTDDS_STATISTICS

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD3(new_change, CacheChange_t* (
            const std::function<uint32_t()>&,
            ChangeKind_t,
            InstanceHandle_t));

    MOCK_METHOD2(new_change, CacheChange_t* (
            ChangeKind_t,
            InstanceHandle_t));

    MOCK_METHOD2(new_change, CacheChange_t* (
            const std::function<uint32_t()>&,
            ChangeKind_t));

    MOCK_METHOD1(release_change, void(CacheChange_t*));

    MOCK_METHOD1(set_separate_sending, void(bool));

    MOCK_METHOD0(getRTPSParticipant, RTPSParticipantImpl* ());

    MOCK_METHOD0 (getTypeMaxSerialized, uint32_t());

    MOCK_METHOD1(calculateMaxDataSize, uint32_t(uint32_t));

    MOCK_METHOD0(getMaxDataSize, uint32_t ());

    MOCK_CONST_METHOD0(get_liveliness_kind, const LivelinessQosPolicyKind& ());

    MOCK_CONST_METHOD0(get_liveliness_lease_duration, const Duration_t& ());

    MOCK_METHOD4(deliver_sample_nts, DeliveryRetCode(
            CacheChange_t*,
            RTPSMessageGroup&,
            LocatorSelectorSender&,
            const std::chrono::time_point<std::chrono::steady_clock>&));

    MOCK_METHOD3(send_nts, bool(
            CDRMessage_t*,
            const LocatorSelectorSender&,
            std::chrono::steady_clock::time_point&));

    MOCK_CONST_METHOD0(is_datasharing_compatible, bool());

    MOCK_CONST_METHOD1(is_datasharing_compatible_with, bool(
            const ReaderProxyData& rdata));

    MOCK_METHOD1(reader_data_filter, void(
            fastdds::rtps::IReaderDataFilter* filter));

    // *INDENT-ON*

    const GUID_t& getGuid() const
    {
        return m_guid;
    }

    EndpointAttributes& getAttributes()
    {
        return m_att.endpoint;
    }

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

    virtual bool wait_for_acknowledgement(
            const SequenceNumber_t&,
            const std::chrono::steady_clock::time_point&,
            std::unique_lock<RecursiveTimedMutex>&)
    {
        return true;
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
            bool& result,
            fastdds::rtps::VendorId_t origin_vendor_id = c_VendorId_Unknown)
    {
        static_cast<void>(writer_guid);
        static_cast<void>(reader_guid);
        static_cast<void>(ack_count);
        static_cast<void>(sn_set);
        static_cast<void>(final_flag);
        static_cast<void>(origin_vendor_id);

        result = false;
        return true;
    }

    virtual bool process_nack_frag(
            const GUID_t& writer_guid,
            const GUID_t& reader_guid,
            uint32_t ack_count,
            const SequenceNumber_t& seq_num,
            const FragmentNumberSet_t fragments_state,
            bool& result,
            fastdds::rtps::VendorId_t origin_vendor_id = c_VendorId_Unknown)
    {
        static_cast<void>(reader_guid);
        static_cast<void>(ack_count);
        static_cast<void>(seq_num);
        static_cast<void>(fragments_state);
        static_cast<void>(origin_vendor_id);

        result = false;
        return writer_guid == m_guid;
    }

    LocatorSelectorSender& get_general_locator_selector()
    {
        return general_locator_selector_;
    }

    LocatorSelectorSender& get_async_locator_selector()
    {
        return async_locator_selector_;
    }

    WriterHistory* history_;

    WriterListener* listener_;

    GUID_t m_guid;

    WriterAttributes m_att;

    LivelinessLostStatus liveliness_lost_status_;

    LocatorSelectorSender general_locator_selector_;

    LocatorSelectorSender async_locator_selector_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_RTPS_RTPSWRITER_H_

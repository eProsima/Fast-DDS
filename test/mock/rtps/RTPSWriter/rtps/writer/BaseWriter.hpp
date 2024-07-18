// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BaseWriter.hpp
 */

#ifndef RTPS_WRITER__BASEWRITER_HPP
#define RTPS_WRITER__BASEWRITER_HPP

#include <gmock/gmock.h>

#include <fastdds/rtps/writer/RTPSWriter.hpp>

#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/messages/RTPSMessageGroup.hpp>
#include <rtps/writer/DeliveryRetCode.hpp>
#include <rtps/writer/LocatorSelectorSender.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class BaseWriter : public RTPSWriter
{

public:

    BaseWriter()
        : general_locator_selector_(*this, ResourceLimitedContainerConfig())
        , async_locator_selector_(*this, ResourceLimitedContainerConfig())
    {
    }

    virtual ~BaseWriter() = default;

    bool matched_reader_add(
            const SubscriptionBuiltinTopicData& wdata) final
    {
        static_cast<void>(wdata);
        return false;
    }

    virtual bool matched_reader_add_edp(
            const ReaderProxyData& wdata)
    {
        static_cast<void>(wdata);
        return false;
    }

    virtual WriterHistory* get_history() const
    {
        return history_;
    }

    static BaseWriter* downcast(
            RTPSWriter* writer)
    {
        return static_cast<BaseWriter*>(writer);
    }

    static BaseWriter* downcast(
            fastdds::rtps::Endpoint* endpoint)
    {
        return static_cast<BaseWriter*>(endpoint);
    }

    RTPSParticipantImpl* get_participant_impl()
    {
        return nullptr;
    }

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD0(get_max_allowed_payload_size, uint32_t());

    MOCK_METHOD4(deliver_sample_nts, DeliveryRetCode(
            CacheChange_t*,
            RTPSMessageGroup&,
            LocatorSelectorSender&,
            const std::chrono::time_point<std::chrono::steady_clock>&));

    MOCK_METHOD4(send_nts, bool(
            const std::vector<eprosima::fastdds::rtps::NetworkBuffer>&,
            const uint32_t&,
            const LocatorSelectorSender&,
            std::chrono::steady_clock::time_point&));

    MOCK_CONST_METHOD0(get_liveliness_kind, const fastdds::dds::LivelinessQosPolicyKind& ());

    MOCK_CONST_METHOD0(get_liveliness_lease_duration, const dds::Duration_t& ());

    MOCK_CONST_METHOD0(is_datasharing_compatible, bool());

    MOCK_CONST_METHOD1(is_datasharing_compatible_with, bool(
            const dds::DataSharingQosPolicy& rdata));

    // *INDENT-ON*

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
            const FragmentNumberSet_t& fragments_state,
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

    LocatorSelectorSender& get_general_locator_selector()
    {
        return general_locator_selector_;
    }

    LocatorSelectorSender& get_async_locator_selector()
    {
        return async_locator_selector_;
    }

    LocatorSelectorSender general_locator_selector_;

    LocatorSelectorSender async_locator_selector_;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // RTPS_WRITER__BASEWRITER_HPP

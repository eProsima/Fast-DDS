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

#ifndef FASTDDS_RTPS_WRITER__RTPSWRITER_HPP
#define FASTDDS_RTPS_WRITER__RTPSWRITER_HPP

#include <condition_variable>
#include <vector>

#include <gmock/gmock.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>
#include <fastdds/rtps/Endpoint.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class WriterHistory;
class RTPSParticipantImpl;

class RTPSWriter : public Endpoint
{
public:

    RTPSWriter()
    {
        static uint8_t entity_id = 0;
        // Generate a guid.
        m_guid.entityId.value[3] = ++entity_id;
    }

    virtual ~RTPSWriter() = default;

    virtual bool matched_reader_add(
            const SubscriptionBuiltinTopicData&) = 0;

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

    WriterListener* get_listener() const
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

    virtual bool matched_readers_guids(
            std::vector<GUID_t>&) const
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
    MOCK_METHOD1(reader_data_filter, void(
            IReaderDataFilter* filter));

    // *INDENT-ON*

    const GUID_t& getGuid() const
    {
        return m_guid;
    }

    EndpointAttributes& getAttributes()
    {
        return m_att.endpoint;
    }

    virtual void update_attributes(
            const WriterAttributes&)
    {
    }

    virtual void send_any_unsent_changes()
    {
    }

    virtual bool is_acked_by_all(
            const SequenceNumber_t& /*seq_num*/) const
    {
        return false;
    }

    virtual bool wait_for_all_acked(
            const dds::Duration_t& /*max_wait*/)
    {
        return true;
    }

    WriterHistory* history_ = nullptr;

    WriterListener* listener_ = nullptr;

    GUID_t m_guid;

    WriterAttributes m_att;

    LivelinessLostStatus liveliness_lost_status_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_WRITER__RTPSWRITER_HPP

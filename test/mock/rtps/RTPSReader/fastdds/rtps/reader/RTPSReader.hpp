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
 * @file RTPSReader.hpp
 */

#ifndef FASTDDS_RTPS_READER__RTPSREADER_HPP
#define FASTDDS_RTPS_READER__RTPSREADER_HPP

#include <gmock/gmock.h>

#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>
#include <fastdds/rtps/Endpoint.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/reader/ReaderListener.hpp>

#include <rtps/builtin/data/WriterProxyData.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class ResourceEvent;

class RTPSReader : public Endpoint
{
public:

    RTPSReader()
    {
    }

    RTPSReader(
            ReaderHistory* history,
            RecursiveTimedMutex* mutex)
    {
        history->mp_reader = this;
        history->mp_mutex = mutex;
    }

    virtual ~RTPSReader() = default;

    virtual bool matched_writer_add(
            const PublicationBuiltinTopicData& wdata) = 0;

    virtual bool matched_writer_remove(
            const GUID_t& wdata,
            bool removed_by_lease = false) = 0;

    virtual bool matched_writer_is_matched(
            const GUID_t& wguid) = 0;

    virtual void assert_writer_liveliness(
            const GUID_t& wguid) = 0;

    virtual bool matched_writers_guids(
            std::vector<GUID_t>&) const
    {
        return false;
    }

    virtual bool is_in_clean_state() = 0;

    virtual ReaderListener* get_listener() const = 0;

    virtual bool set_listener(
            ReaderListener* listener) = 0;

    ReaderHistory* get_history()
    {
        get_history_mock();
        return history_;
    }

    virtual CacheChange_t* next_unread_cache()
    {
        return nullptr;
    }

    virtual CacheChange_t* next_untaken_cache()
    {
        return nullptr;
    }

    virtual bool is_sample_valid(
            const void* /*data*/,
            const GUID_t& /*writer*/,
            const SequenceNumber_t& /*sn*/) const
    {
        return true;
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

    virtual void set_enabled_statistics_writers_mask(
            uint32_t /*enabled_writers*/)
    {
    }

    template<typename T>
    bool get_connections(
            T& /*connection_list*/)
    {
        return true;
    }

#endif // FASTDDS_STATISTICS

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD0(expects_inline_qos, bool());

    MOCK_METHOD0(get_history_mock, ReaderHistory* ());

    MOCK_CONST_METHOD0(get_content_filter, eprosima::fastdds::rtps::IReaderDataFilter* ());

    MOCK_METHOD1(set_content_filter, void (eprosima::fastdds::rtps::IReaderDataFilter* filter));

    MOCK_METHOD1(wait_for_unread_cache, bool (const eprosima::fastdds::dds::Duration_t& timeout));

    MOCK_CONST_METHOD0(get_unread_count, uint64_t());

    MOCK_METHOD1(get_unread_count, uint64_t(bool));

    // *INDENT-ON*


    void set_history(
            ReaderHistory* history)
    {
        history->mp_reader = this;
        history->mp_mutex = &mp_mutex;
        history_ = history;
    }

    ReaderHistory* history_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_READER__RTPSREADER_HPP

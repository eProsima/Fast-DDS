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
 * @file RTPSReader.h
 */

#ifndef _FASTDDS_RTPS_READER_RTPSREADER_H_
#define _FASTDDS_RTPS_READER_RTPSREADER_H_

#include <fastdds/rtps/writer/IReaderDataFilter.hpp>

#include <fastrtps/rtps/Endpoint.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/rtps/attributes/WriterAttributes.h>
#include <fastrtps/rtps/attributes/ReaderAttributes.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>

#include <gmock/gmock.h>

namespace eprosima {
namespace fastrtps {
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
            const WriterProxyData& wdata) = 0;

    virtual bool matched_writer_remove(
            const GUID_t& wdata,
            bool removed_by_lease = false) = 0;

    virtual bool matched_writer_is_matched(
            const GUID_t& wguid) = 0;

    const GUID_t& getGuid()
    {
        return m_guid;
    }

    ReaderListener* getListener() const
    {
        return listener_;
    }

    bool setListener(
            ReaderListener* listener)
    {
        listener_ = listener;
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

#endif // FASTDDS_STATISTICS

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD1(change_removed_by_history, bool(CacheChange_t* change));

    MOCK_METHOD0(getHistory_mock, ReaderHistory* ());

    MOCK_METHOD2(reserveCache, bool (CacheChange_t** a_change, uint32_t dataCdrSerializedSize));

    MOCK_METHOD1(releaseCache, void (CacheChange_t* a_change));

    MOCK_METHOD0(expectsInlineQos, bool());

    MOCK_METHOD1(wait_for_unread_cache, bool (const eprosima::fastrtps::Duration_t& timeout));

    MOCK_METHOD0(get_unread_count, uint64_t());

    MOCK_METHOD1(set_content_filter, void (eprosima::fastdds::rtps::IReaderDataFilter* filter));

    // *INDENT-ON*


    virtual bool processDataMsg(
            CacheChange_t*)
    {
        return true;
    }

    virtual bool processDataFragMsg(
            CacheChange_t*,
            uint32_t,
            uint32_t,
            uint16_t)
    {
        return true;
    }

    virtual bool processHeartbeatMsg(
            const GUID_t&,
            uint32_t,
            const SequenceNumber_t&,
            const SequenceNumber_t&,
            bool,
            bool)
    {
        return true;
    }

    virtual bool processGapMsg(
            const GUID_t&,
            const SequenceNumber_t&,
            const SequenceNumberSet_t&)
    {
        return true;
    }

    virtual bool change_removed_by_history(
            CacheChange_t*,
            WriterProxy*)
    {
        return true;
    }

    virtual bool nextUnreadCache(
            CacheChange_t**,
            WriterProxy**)
    {
        return true;
    }

    virtual bool nextUntakenCache(
            CacheChange_t**,
            WriterProxy**)
    {
        return true;
    }

    virtual bool isInCleanState()
    {
        return true;
    }

    ReaderHistory* getHistory()
    {
        getHistory_mock();
        return history_;
    }

    bool is_sample_valid(
            const void* /*data*/,
            const GUID_t& /*writer*/,
            const SequenceNumber_t& /*sn*/) const
    {
        return true;
    }

    virtual bool begin_sample_access_nts(
            CacheChange_t* /*change*/,
            WriterProxy*& /*wp*/,
            bool& /*is_future_change*/)
    {
        return true;
    }

    virtual void end_sample_access_nts(
            CacheChange_t* /*change*/,
            WriterProxy*& /*wp*/,
            bool /*mark_as_read*/)
    {
    }

    virtual void change_read_by_user(
            CacheChange_t* /*change*/,
            const WriterProxy* /*writer*/,
            bool mark_as_read = true)
    {
        (void) mark_as_read;
    }

    ReaderHistory* history_;

    ReaderListener* listener_;

    const GUID_t m_guid;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_RTPS_READER_RTPSREADER_H_

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
 * @file ReaderHistory.h
 */

#ifndef _FASTDDS_RTPS_READERHISTORY_H_
#define _FASTDDS_RTPS_READERHISTORY_H_

#include <fastrtps/rtps/common/CacheChange.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/utils/TimedMutex.hpp>

#include <mutex>

#include <gmock/gmock.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSReader;
class WriterProxy;

class ReaderHistory
{
    friend class RTPSReader;

public:

    using iterator = std::vector<CacheChange_t*>::iterator;
    using const_iterator = std::vector<CacheChange_t*>::const_iterator;

    ReaderHistory(
            const HistoryAttributes& /*att*/)
    {
    }

    virtual ~ReaderHistory()
    {
    }

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD1(remove_change_mock, bool(CacheChange_t*));

    MOCK_METHOD0(getHistorySize, size_t());

    MOCK_METHOD1(get_earliest_change, bool(
            CacheChange_t** change));

    MOCK_METHOD1(add_change_mock, bool(CacheChange_t*));
    // *INDENT-ON*

    bool add_change(
            CacheChange_t* change)
    {
        bool ret = add_change_mock(change);
        samples_number_mutex_.lock();
        ++samples_number_;
        change->sequenceNumber = ++last_sequence_number_;
        samples_number_mutex_.unlock();
        return ret;
    }

    virtual bool can_change_be_added_nts(
            const GUID_t&,
            uint32_t,
            size_t,
            bool&) const
    {
        return true;
    }

    virtual bool received_change(
            CacheChange_t*,
            size_t)
    {
        return true;
    }

    virtual bool completed_change(
            rtps::CacheChange_t*)
    {
        return true;
    }

    bool remove_change(
            CacheChange_t* change)
    {
        bool ret = remove_change_mock(change);
        delete change;
        return ret;
    }

    inline RecursiveTimedMutex* getMutex()
    {
        return mp_mutex;
    }

    const_iterator find_change_nts(
            CacheChange_t* change)
    {
        return std::find(m_changes.cbegin(), m_changes.cend(), change);
    }

    const_iterator changesBegin() const
    {
        return m_changes.cbegin();
    }

    const_iterator changesEnd() const
    {
        return m_changes.cend();
    }

    virtual iterator remove_change_nts(
            const_iterator removal,
            bool release = true)
    {
        (void)release;
        return m_changes.erase(removal);
    }

    virtual void writer_unmatched(
            const GUID_t& /*writer_guid*/,
            const SequenceNumber_t& /*last_notified_seq*/)
    {
    }

    HistoryAttributes m_att;

protected:

    template<typename Pred>
    inline void remove_changes_with_pred(
            Pred)
    {
    }

    RTPSReader* mp_reader;
    RecursiveTimedMutex* mp_mutex;
    std::vector<CacheChange_t*> m_changes;
    bool m_isHistoryFull;
    std::mutex samples_number_mutex_;
    unsigned int samples_number_;
    SequenceNumber_t last_sequence_number_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_RTPS_READERHISTORY_H_

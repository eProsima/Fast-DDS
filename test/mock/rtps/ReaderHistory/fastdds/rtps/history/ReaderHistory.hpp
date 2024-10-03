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
 * @file ReaderHistory.hpp
 */

#ifndef FASTDDS_RTPS_HISTORY__READERHISTORY_HPP
#define FASTDDS_RTPS_HISTORY__READERHISTORY_HPP

#include <mutex>

#include <gmock/gmock.h>

#include <fastdds/dds/core/status/SampleRejectedStatus.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/utils/TimedMutex.hpp>

namespace eprosima {
namespace fastdds {
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

    MOCK_METHOD3(get_change, bool(
            const SequenceNumber_t& seq,
            const GUID_t& guid,
            CacheChange_t** change));

    MOCK_METHOD1(get_earliest_change, bool(
            CacheChange_t** change));

    MOCK_METHOD1(add_change_mock, bool(CacheChange_t*));

    MOCK_METHOD1(find_change, const_iterator(CacheChange_t*));

    MOCK_METHOD2(remove_change, iterator(const_iterator, bool));

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

    virtual bool received_change(
            CacheChange_t*,
            size_t,
            fastdds::dds::SampleRejectedStatusKind&)
    {
        return true;
    }

    virtual bool completed_change(
            rtps::CacheChange_t*)
    {
        return true;
    }

    virtual bool completed_change(
            rtps::CacheChange_t*,
            size_t,
            fastdds::dds::SampleRejectedStatusKind&)
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

    inline RecursiveTimedMutex* getMutex() const
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

    iterator changesEnd()
    {
        return m_changes.end();
    }

    virtual iterator remove_change_nts(
            const_iterator removal,
            bool release = true)
    {
        (void)release;
        return m_changes.erase(removal);
    }

    virtual iterator remove_change_nts(
            const_iterator removal,
            const std::chrono::time_point<std::chrono::steady_clock>&,
            bool release = true)
    {
        return remove_change_nts(removal, release);
    }

    virtual void writer_unmatched(
            const GUID_t& /*writer_guid*/,
            const SequenceNumber_t& /*last_notified_seq*/)
    {
    }

    virtual void writer_update_its_ownership_strength_nts(
            const GUID_t& writer_guid,
            const uint32_t ownership_strength)
    {
        static_cast<void>(writer_guid);
        static_cast<void>(ownership_strength);
    }

    bool matches_change(
            const CacheChange_t* inner_change,
            CacheChange_t* outer_change)
    {
        return inner_change->sequenceNumber == outer_change->sequenceNumber &&
               inner_change->writerGUID == outer_change->writerGUID;
    }

    iterator remove_iterator_constness(
            const_iterator c_it)
    {
        iterator it = m_changes.begin();
        std::advance(it, std::distance<const_iterator>(m_changes.cbegin(), c_it));
        return it;
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
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_HISTORY__READERHISTORY_HPP

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
 * @file WriterHistory.hpp
 */

#ifndef FASTDDS_RTPS_HISTORY__WRITERHISTORY_HPP
#define FASTDDS_RTPS_HISTORY__WRITERHISTORY_HPP

#include <condition_variable>

#include <gmock/gmock.h>

#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/utils/TimedMutex.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class IChangePool;
class IPayloadPool;
class ReaderProxy;
class BaseWriter;

class WriterHistory
{
public:

    WriterHistory(
            const HistoryAttributes& /*att*/)
        : samples_number_(0)
    {
    }

    WriterHistory(
            const HistoryAttributes& /*att*/,
            const std::shared_ptr<IPayloadPool>&)
        : samples_number_(0)
    {
    }

    WriterHistory(
            const HistoryAttributes& /*att*/,
            const std::shared_ptr<IPayloadPool>&,
            const std::shared_ptr<IChangePool>&)
        : samples_number_(0)
    {
    }

    WriterHistory()
        : samples_number_(0)
    {
    }

    virtual ~WriterHistory() = default;

    using iterator = std::vector<CacheChange_t*>::iterator;

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD2(create_change, CacheChange_t* (
            ChangeKind_t,
            InstanceHandle_t));

    CacheChange_t* create_change(
            uint32_t size,
            ChangeKind_t kind)
    {
        return create_change(size, kind, InstanceHandle_t{});
    }

    MOCK_METHOD3(create_change, CacheChange_t* (
            uint32_t,
            ChangeKind_t,
            InstanceHandle_t handle));

    MOCK_METHOD1(release_change, bool(CacheChange_t*));

    MOCK_METHOD1(add_change_mock, bool(CacheChange_t*));

    MOCK_METHOD2(add_change, bool(CacheChange_t * a_change, WriteParams & wparams));
    // *INDENT-ON*

    bool add_change(
            CacheChange_t* change)
    {
        bool ret = add_change_mock(change);
        samples_number_mutex_.lock();
        ++samples_number_;
        change->sequenceNumber = ++last_sequence_number_;
        samples_number_mutex_.unlock();
        samples_number_cond_.notify_all();
        return ret;
    }

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD3(get_change, bool(
            const SequenceNumber_t& seq,
            const GUID_t& guid,
            CacheChange_t** change));

    MOCK_METHOD1(get_earliest_change, bool(
            CacheChange_t** change));

    MOCK_METHOD1(remove_change, bool(const SequenceNumber_t&));

    MOCK_METHOD1(remove_change_and_reuse, CacheChange_t*(const SequenceNumber_t&));

    MOCK_METHOD1(remove_change_mock, bool(CacheChange_t*));

    MOCK_METHOD2(remove_change_mock, bool(CacheChange_t*,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time));

    MOCK_METHOD0(getHistorySize, size_t());

    MOCK_METHOD0(remove_min_change, bool());

    MOCK_METHOD1(remove_min_change, bool(
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time));

    MOCK_METHOD3(add_change_, bool(
            CacheChange_t* a_change,
            WriteParams &wparams,
            std::chrono::time_point<std::chrono::steady_clock> max_blocking_time));

    MOCK_METHOD2(add_change_, bool(
            CacheChange_t* a_change,
            WriteParams &wparams));

    MOCK_METHOD4(add_change_, bool(
            CacheChange_t* a_change,
            WriteParams &wparams,
            void* pre_commit,
            std::chrono::time_point<std::chrono::steady_clock> max_blocking_time));

    // *INDENT-ON*

    template<typename PreCommitHook>
    bool add_change_with_commit_hook(
            CacheChange_t* a_change,
            WriteParams& wparams,
            PreCommitHook pre_commit,
            std::chrono::time_point<std::chrono::steady_clock> max_blocking_time)
    {
        return add_change_(a_change, wparams, &pre_commit, max_blocking_time);
    }

    MOCK_METHOD1(set_fragments, void(
                CacheChange_t * change));

    bool remove_change(
            CacheChange_t* change)
    {
        bool ret = remove_change_mock(change);
        delete change;
        return ret;
    }

    bool remove_change(
            CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
    {
        bool ret = remove_change_mock(change, max_blocking_time);
        delete change;
        return ret;
    }

    virtual bool remove_change_g(
            fastdds::rtps::CacheChange_t* a_change)
    {
        return remove_change(a_change);
    }

    virtual bool remove_change_g(
            fastdds::rtps::CacheChange_t* a_change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
    {
        return remove_change(a_change, max_blocking_time);
    }

    void wait_for_more_samples_than(
            unsigned int minimum)
    {
        std::unique_lock<std::mutex> lock(samples_number_mutex_);

        if (samples_number_ <= minimum)
        {
            samples_number_cond_.wait(lock, [&]()
                    {
                        return samples_number_ > minimum;
                    });
        }
    }

    SequenceNumber_t next_sequence_number() const
    {
        return last_sequence_number_ + 1;
    }

    std::vector<CacheChange_t*>::iterator changesBegin()
    {
        return m_changes.begin();
    }

    std::vector<CacheChange_t*>::reverse_iterator changesRbegin()
    {
        return m_changes.rbegin();
    }

    std::vector<CacheChange_t*>::iterator changesEnd()
    {
        return m_changes.end();
    }

    std::vector<CacheChange_t*>::reverse_iterator changesRend()
    {
        return m_changes.rend();
    }

    RecursiveTimedMutex* getMutex() const
    {
        return mp_mutex;
    }

    inline uint32_t getTypeMaxSerialized()
    {
        return m_att.payloadMaxSize;
    }

    HistoryAttributes m_att;
    std::vector<CacheChange_t*> m_changes;

    std::condition_variable samples_number_cond_;
    std::mutex samples_number_mutex_;
    unsigned int samples_number_;
    SequenceNumber_t last_sequence_number_;
    RecursiveTimedMutex* mp_mutex;
    bool m_isHistoryFull;
    BaseWriter* mp_writer;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_HISTORY__WRITERHISTORY_HPP

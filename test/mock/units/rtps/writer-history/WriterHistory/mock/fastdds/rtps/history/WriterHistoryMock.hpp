// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_TEST_MOCK_FASTDDS_RTPS_HISTORY__WRITERHISTORYMOCK_HPP
#define FASTDDS_TEST_MOCK_FASTDDS_RTPS_HISTORY__WRITERHISTORYMOCK_HPP

#include <condition_variable>
#include <cstdint>
#include <mutex>

#include <gmock/gmock.h>

#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/WriteParams.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/utils/TimedMutex.hpp>
#include <mock/MockCompanion.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Expectations of a WriterHistory created with the mock implementation, reached with mock::mock_of().
 *
 * Changes added successfully are kept in the history with the next sequence number, so they can be found and removed
 * later. Changes are never released to any pool: the changes returned by create_change() belong to the test.
 */
class WriterHistoryMock
{
public:

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD2(create_change, CacheChange_t* (
            ChangeKind_t,
            InstanceHandle_t));

    MOCK_METHOD3(create_change, CacheChange_t* (
            uint32_t,
            ChangeKind_t,
            InstanceHandle_t));

    MOCK_METHOD1(add_change, bool(CacheChange_t*));

    MOCK_METHOD2(add_change, bool(CacheChange_t*, WriteParams&));

    //! Called when a change in the history is removed, whatever the method used.
    MOCK_METHOD1(remove_change, bool(CacheChange_t*));

    MOCK_METHOD1(remove_change, bool(const SequenceNumber_t&));

    MOCK_METHOD1(remove_change_and_reuse, CacheChange_t*(const SequenceNumber_t&));

    MOCK_METHOD0(remove_min_change, bool());

    MOCK_METHOD1(release_change, bool(CacheChange_t*));
    // *INDENT-ON*

    /**
     * Wait until more than a number of changes have been added successfully to the history.
     * @param minimum Number of changes that must be exceeded.
     */
    void wait_for_more_samples_than(
            unsigned int minimum)
    {
        std::unique_lock<std::mutex> lock(samples_number_mutex_);
        samples_number_cond_.wait(lock, [&]()
                {
                    return samples_number_ > minimum;
                });
    }

    /**
     * Count a change added successfully to the history.
     */
    void sample_added()
    {
        {
            std::lock_guard<std::mutex> lock(samples_number_mutex_);
            ++samples_number_;
        }
        samples_number_cond_.notify_all();
    }

    //! Mutex of the history, used until a writer sets its own.
    RecursiveTimedMutex mutex;

private:

    std::mutex samples_number_mutex_;
    std::condition_variable samples_number_cond_;
    unsigned int samples_number_ = 0;
};

} // namespace rtps

namespace mock {

template<>
struct MockOf<rtps::WriterHistory>
{
    using type = rtps::WriterHistoryMock;
};

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_FASTDDS_RTPS_HISTORY__WRITERHISTORYMOCK_HPP

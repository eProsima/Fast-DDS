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

#ifndef FASTDDS_TEST_MOCK_FASTDDS_RTPS_HISTORY__READERHISTORYMOCK_HPP
#define FASTDDS_TEST_MOCK_FASTDDS_RTPS_HISTORY__READERHISTORYMOCK_HPP

#include <gmock/gmock.h>

#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/utils/TimedMutex.hpp>
#include <mock/MockCompanion.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Expectations of a ReaderHistory created with the mock implementation, reached with mock::mock_of().
 *
 * Changes are added to the history as in the real one, without the reader being involved. Removed changes are never
 * released to the reader: they belong to the test.
 */
class ReaderHistoryMock
{
public:

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    /**
     * Called when a change in the history is removed, whatever the method used.
     * The returned value is ignored: as in the real history, the change is always removed, so loops removing changes
     * always advance.
     */
    MOCK_METHOD1(remove_change, bool(CacheChange_t*));
    // *INDENT-ON*

    //! Mutex of the history, used until a reader sets its own.
    RecursiveTimedMutex mutex;
};

} // namespace rtps

namespace mock {

template<>
struct MockOf<rtps::ReaderHistory>
{
    using type = rtps::ReaderHistoryMock;
};

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_FASTDDS_RTPS_HISTORY__READERHISTORYMOCK_HPP

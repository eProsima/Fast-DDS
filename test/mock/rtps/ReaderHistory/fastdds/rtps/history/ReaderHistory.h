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

    ReaderHistory(
            const HistoryAttributes& /*att*/)
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

protected:

    RTPSReader* mp_reader;
    RecursiveTimedMutex* mp_mutex;
    std::vector<CacheChange_t*> m_changes;
    bool m_isHistoryFull;
    std::mutex samples_number_mutex_;
    unsigned int samples_number_;
    SequenceNumber_t last_sequence_number_;
    HistoryAttributes m_att;

};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_RTPS_READERHISTORY_H_

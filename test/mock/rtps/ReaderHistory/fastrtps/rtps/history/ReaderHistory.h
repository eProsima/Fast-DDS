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

#ifndef _RTPS_HISTORY_READERHISTORY_H_
#define _RTPS_HISTORY_READERHISTORY_H_

#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/utils/TimedMutex.hpp>

#include <gmock/gmock.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSReader;

class ReaderHistory
{
    friend class RTPSReader;

public:

    ReaderHistory(
            const HistoryAttributes& /*att*/)
    {
    }

    MOCK_METHOD1(remove_change_mock, bool(CacheChange_t*));

    bool remove_change(
            CacheChange_t* change)
    {
        bool ret = remove_change_mock(change);
        delete change;
        return ret;
    }

protected:

    RTPSReader* mp_reader;
    RecursiveTimedMutex* mp_mutex;
    HistoryAttributes m_att;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _RTPS_HISTORY_READERHISTORY_H_

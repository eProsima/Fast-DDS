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

class RTPSReader : public Endpoint
{
    public:

        RTPSReader() {}

        RTPSReader(ReaderHistory* history, RecursiveTimedMutex* mutex)
        {
            history->mp_reader = this;
            history->mp_mutex = mutex;
        }

        virtual ~RTPSReader() = default;


        virtual bool matched_writer_add(const WriterProxyData& wdata) = 0;

        virtual bool matched_writer_remove(const GUID_t& wdata) = 0;

        MOCK_METHOD1(change_removed_by_history, bool(CacheChange_t* change));

        MOCK_METHOD0(getHistory_mock, ReaderHistory*());

        MOCK_CONST_METHOD0(getGuid, const GUID_t&());

        ReaderHistory* getHistory()
        {
            getHistory_mock();
            return history_;
        }

        ReaderHistory* history_;

        ReaderListener* listener_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_RTPS_READER_RTPSREADER_H_

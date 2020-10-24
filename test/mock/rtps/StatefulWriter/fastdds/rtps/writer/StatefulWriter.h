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
 * @file StatefulWriter.h
 */

#ifndef _FASTDDS_RTPS_STATEFULWRITER_H_
#define _FASTDDS_RTPS_STATEFULWRITER_H_

#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/writer/IReaderDataFilter.hpp>
#include <fastrtps/rtps/history/WriterHistory.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSParticipantImpl;
class ReaderProxy;

class StatefulWriter : public RTPSWriter
{
public:

    StatefulWriter(
            RTPSParticipantImpl* participant)
        : participant_(participant)
        , mp_history(new WriterHistory())
    {
    }

    StatefulWriter()
        : participant_(nullptr)
        , mp_history(new WriterHistory())
    {
    }

    virtual ~StatefulWriter()
    {
        delete mp_history;
    }

    MOCK_METHOD1(matched_reader_add, bool(const ReaderProxyData&));

    MOCK_METHOD1(matched_reader_remove, bool(const GUID_t&));

    MOCK_METHOD0(getGuid, const GUID_t& ());

    MOCK_METHOD1(unsent_change_added_to_history_mock, void(CacheChange_t*));

    MOCK_METHOD1(perform_nack_supression, void(const GUID_t&));

    MOCK_METHOD1(intraprocess_heartbeat, void(const ReaderProxy*));

    MOCK_METHOD2(intraprocess_gap, void(const ReaderProxy*, const SequenceNumber_t&));

    RTPSParticipantImpl* getRTPSParticipant()
    {
        return participant_;
    }

    SequenceNumber_t get_seq_num_min()
    {
        return SequenceNumber_t(0, 0);
    }

    SequenceNumber_t next_sequence_number() const
    {
        return mp_history->next_sequence_number();
    }

    void reader_data_filter(
            fastdds::rtps::IReaderDataFilter* reader_data_filter)
    {
        reader_data_filter_ = reader_data_filter;
    }

    const fastdds::rtps::IReaderDataFilter* reader_data_filter() const
    {
        return reader_data_filter_;
    }

private:

    friend class ReaderProxy;

    RTPSParticipantImpl* participant_;

    WriterHistory* mp_history;

    fastdds::rtps::IReaderDataFilter* reader_data_filter_;

};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_RTPS_STATEFULWRITER_H_

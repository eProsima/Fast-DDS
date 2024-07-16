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
 * @file StatefulWriter.hpp
 */

#ifndef FASTDDS_RTPS_WRITER__STATEFULWRITER_HPP
#define FASTDDS_RTPS_WRITER__STATEFULWRITER_HPP

#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>

#include <rtps/writer/BaseWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSParticipantImpl;
class ReaderProxy;

class StatefulWriter : public BaseWriter
{
public:

    StatefulWriter(
            RTPSParticipantImpl* participant)
        : participant_(participant)
        , mp_history(new WriterHistory())
    {
        mp_history->m_att.initialReservedCaches = 0;
    }

    StatefulWriter()
        : StatefulWriter(nullptr)
    {
    }

    virtual ~StatefulWriter()
    {
        delete mp_history;
    }

    MOCK_METHOD1(matched_reader_add_edp, bool(const ReaderProxyData&));

    MOCK_METHOD1(matched_reader_remove, bool(const GUID_t&));

    MOCK_METHOD1 (matched_reader_is_matched, bool(const GUID_t& reader_guid));

    MOCK_METHOD1(unsent_change_added_to_history_mock, void(CacheChange_t*));

    MOCK_METHOD1(perform_nack_supression, void(const GUID_t&));

    MOCK_METHOD1(intraprocess_heartbeat, void(const ReaderProxy*));

    MOCK_METHOD2(intraprocess_gap, void(const ReaderProxy*, const SequenceNumber_t&));

    MOCK_METHOD2(send_periodic_heartbeat, bool(
                bool final,
                bool liveliness));

    MOCK_METHOD1(send_periodic_heartbeat, bool(
                bool final));

    MOCK_METHOD0(send_periodic_heartbeat, bool());


    RTPSParticipantImpl* getRTPSParticipant()
    {
        return participant_;
    }

    SequenceNumber_t get_seq_num_min()
    {
        return SequenceNumber_t::unknown();
    }

    WriterHistory* get_history() const override
    {
        return history_ ? history_ : mp_history;
    }

    SequenceNumber_t next_sequence_number() const
    {
        return get_history()->next_sequence_number();
    }

    void reader_data_filter(
            IReaderDataFilter* filter)
    {
        reader_data_filter_ = filter;
    }

    const IReaderDataFilter* reader_data_filter() const
    {
        return reader_data_filter_;
    }

    bool get_disable_positive_acks() const override
    {
        return false;
    }

    bool has_been_fully_delivered(
            const SequenceNumber_t& /*seq_num*/) const override
    {
        return false;
    }

private:

    friend class ReaderProxy;

    RTPSParticipantImpl* participant_;

    WriterHistory* mp_history;

    IReaderDataFilter* reader_data_filter_;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_WRITER__STATEFULWRITER_HPP

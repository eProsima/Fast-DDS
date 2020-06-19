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
 * @file RTPSWriter.h
 */

#ifndef _FASTDDS_RTPS_RTPSWRITER_H_
#define _FASTDDS_RTPS_RTPSWRITER_H_

#include <fastrtps/rtps/attributes/WriterAttributes.h>
#include <fastrtps/rtps/writer/WriterListener.h>
#include <fastrtps/rtps/Endpoint.h>
#include <fastrtps/rtps/common/CacheChange.h>

#include <condition_variable>
#include <gmock/gmock.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class WriterHistory;
class RTPSParticipantImpl;

class RTPSWriter : public Endpoint
{
public:

    virtual ~RTPSWriter() = default;

    virtual bool matched_reader_add(
            const ReaderProxyData& ratt) = 0;

    virtual bool matched_reader_remove(
            const GUID_t& ratt) = 0;

    virtual bool matched_reader_is_matched(
            const GUID_t& rguid) = 0;

    WriterListener* getListener() const
    {
        return listener_;
    }

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_CONST_METHOD0(getGuid, const GUID_t& ());

    MOCK_METHOD3(new_change, CacheChange_t* (
            const std::function<uint32_t()>&,
            ChangeKind_t,
            InstanceHandle_t));

    MOCK_METHOD1(set_separate_sending, void(bool));

    MOCK_METHOD0(getRTPSParticipant, RTPSParticipantImpl* ());

    MOCK_METHOD0 (getTypeMaxSerialized, uint32_t());
    // *INDENT-ON*

    virtual bool process_acknack(
            const GUID_t& writer_guid,
            const GUID_t& reader_guid,
            uint32_t ack_count,
            const SequenceNumberSet_t& sn_set,
            bool final_flag,
            bool& result)
    {
        (void)writer_guid; (void)reader_guid; (void)ack_count; (void)sn_set; (void)final_flag;

        result = false;
        return true;
    }

    WriterHistory* history_;

    WriterListener* listener_;

    const GUID_t m_guid;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_RTPS_RTPSWRITER_H_

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
 * @file StatelessWriter.h
 */

#ifndef _FASTDDS_RTPS_STATELESSWRITER_H_
#define _FASTDDS_RTPS_STATELESSWRITER_H_

#include <fastrtps/rtps/writer/RTPSWriter.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSParticipantImpl;

class StatelessWriter : public RTPSWriter
{
public:

    StatelessWriter(
            RTPSParticipantImpl* participant)
        : participant_(participant)
    {
    }

    virtual ~StatelessWriter() = default;

    MOCK_METHOD1(matched_reader_add, bool(const ReaderProxyData&));

    MOCK_METHOD1(matched_reader_remove, bool(const GUID_t&));

    MOCK_METHOD1 (matched_reader_is_matched, bool(const GUID_t& reader_guid));

    MOCK_METHOD0(getGuid, const GUID_t& ());

    MOCK_METHOD1(unsent_change_added_to_history_mock, void(CacheChange_t*));

    RTPSParticipantImpl* getRTPSParticipant()
    {
        return participant_;
    }

private:

    RTPSParticipantImpl* participant_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_RTPS_STATELESSWRITER_H_

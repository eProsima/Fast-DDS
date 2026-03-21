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
 * @file StatelessReader.hpp
 */

#ifndef FASTDDS_RTPS_READER__STATELESSREADER_HPP
#define FASTDDS_RTPS_READER__STATELESSREADER_HPP

#include <rtps/reader/BaseReader.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class StatelessReader : public fastdds::rtps::BaseReader
{
public:

    StatelessReader() = default;

    StatelessReader(
            ReaderHistory* history,
            RecursiveTimedMutex* mutex)
        : fastdds::rtps::BaseReader(history, mutex)
    {
    }

    virtual ~StatelessReader() = default;

    MOCK_METHOD1(matched_writer_add_edp, bool(const WriterProxyData&));

    MOCK_METHOD2(matched_writer_remove, bool(const GUID_t&, bool));

    MOCK_METHOD1 (matched_writer_is_matched, bool(const GUID_t& writer_guid));

    MOCK_METHOD1(assert_writer_liveliness, void(const GUID_t& writer_guid));

    MOCK_METHOD0(is_in_clean_state, bool());
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_READER__STATELESSREADER_HPP

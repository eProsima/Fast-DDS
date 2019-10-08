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
 * @file StatelessReader.h
 */

#ifndef _FASTDDS_RTPS_READER_STATELESSREADER_H_
#define _FASTDDS_RTPS_READER_STATELESSREADER_H_

#include <fastrtps/rtps/reader/RTPSReader.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class StatelessReader : public RTPSReader
{
    public:

        virtual ~StatelessReader() = default;

        MOCK_METHOD1(matched_writer_add, bool(const WriterProxyData&));

        MOCK_METHOD1(matched_writer_remove, bool(const GUID_t&));
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_RTPS_READER_STATELESSREADER_H_

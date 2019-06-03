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

#ifndef _RTPS_READER_STATEFULREADER_H_
#define _RTPS_READER_STATEFULREADER_H_

#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/attributes/ReaderAttributes.h>
#include <fastrtps/rtps/common/Guid.h>

namespace eprosima {
namespace fastrtps {
namespace rtps{

class StatefulReader : public RTPSReader
    {
        public:

            StatefulReader() {}

            StatefulReader(ReaderHistory* history, std::recursive_timed_mutex* mutex) : RTPSReader(history, mutex) {}

            virtual ~StatefulReader() {}

            MOCK_METHOD1(matched_writer_add, bool(RemoteWriterAttributes&));

            MOCK_METHOD1(matched_writer_remove, bool(RemoteWriterAttributes&));

            // In real class, inherited from Endpoint base class.
            inline const GUID_t& getGuid() const { return guid_; };

            inline ReaderTimes& getTimes(){return times_;};

        private:

            GUID_t guid_;

            ReaderTimes times_;
    };

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _RTPS_READER_STATEFULREADER_H_

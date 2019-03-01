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

#ifndef _RTPS_WRITER_RTPSWRITER_H_
#define _RTPS_WRITER_RTPSWRITER_H_

#include <fastrtps/rtps/attributes/WriterAttributes.h>
#include <fastrtps/rtps/Endpoint.h>
#include <fastrtps/rtps/common/CacheChange.h>

#include <condition_variable>
#include <gmock/gmock.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class WriterHistory;

class RTPSWriter : public Endpoint
{
    public:

        virtual ~RTPSWriter() = default;

        virtual bool matched_reader_add(const ReaderProxyData& ratt) = 0;

        virtual bool matched_reader_add(const RemoteReaderAttributes& ratt) = 0;

        virtual bool matched_reader_remove(const GUID_t& ratt) = 0;

        MOCK_METHOD3(new_change, CacheChange_t*(const std::function<uint32_t()>&,
            ChangeKind_t, InstanceHandle_t));
			
		MOCK_METHOD1(set_separate_sending, void(bool));

        WriterHistory* history_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _RTPS_WRITER_RTPSWRITER_H_

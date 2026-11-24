// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_TEST_MOCK_RTPS_MESSAGES__RTPSGAPBUILDERMOCK_HPP
#define FASTDDS_TEST_MOCK_RTPS_MESSAGES__RTPSGAPBUILDERMOCK_HPP

#include <gmock/gmock.h>

#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <mock/MockCompanion.hpp>
#include <rtps/messages/RTPSGapBuilder.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Expectations of a RTPSGapBuilder created with the mock implementation, reached with mock::mock_of().
 * No GAP message is added to the message group.
 */
class RTPSGapBuilderMock
{
public:

    MOCK_METHOD1(add, bool(const SequenceNumber_t& gap_sequence));
    MOCK_METHOD0(flush, bool());

};

} // namespace rtps

namespace mock {

template<>
struct MockOf<rtps::RTPSGapBuilder>
{
    using type = rtps::RTPSGapBuilderMock;
};

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_RTPS_MESSAGES__RTPSGAPBUILDERMOCK_HPP

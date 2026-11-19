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

#ifndef FASTDDS_TEST_MOCK_RTPS_MESSAGES__MESSAGERECEIVERMOCK_HPP
#define FASTDDS_TEST_MOCK_RTPS_MESSAGES__MESSAGERECEIVERMOCK_HPP

#include <gmock/gmock.h>

#include <mock/MockCompanion.hpp>
#include <rtps/messages/MessageReceiver.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Expectations of a MessageReceiver created with the mock implementation, reached with mock::mock_of().
 */
class MessageReceiverMock
{
public:

    MOCK_METHOD3(processCDRMsg, void(const Locator_t&, const Locator_t&, CDRMessage_t*));
    MOCK_METHOD1(associateEndpoint, void(Endpoint*));
    MOCK_METHOD1(removeEndpoint, void(Endpoint*));

};

} // namespace rtps

namespace mock {

template<>
struct MockOf<rtps::MessageReceiver>
{
    using type = rtps::MessageReceiverMock;
};

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_RTPS_MESSAGES__MESSAGERECEIVERMOCK_HPP

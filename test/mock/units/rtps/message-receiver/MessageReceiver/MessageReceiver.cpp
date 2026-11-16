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

// Link-seam implementation of MessageReceiver: replaces src/cpp/rtps/messages/MessageReceiver.cpp and forwards every
// public method to the MessageReceiverMock companion of the instance.

#include <rtps/messages/MessageReceiver.h>

#include <mock/rtps/messages/MessageReceiverMock.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Companion = mock::MockCompanion<MessageReceiver>;

MessageReceiver::MessageReceiver(
        RTPSParticipantImpl* participant,
        uint32_t)
#if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
    : participant_(participant)
#endif // if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
{
    static_cast<void>(participant);
    Companion::attach(this);
}

MessageReceiver::~MessageReceiver()
{
    Companion::detach(this);
}

void MessageReceiver::processCDRMsg(
        const Locator_t& source_locator,
        const Locator_t& reception_locator,
        CDRMessage_t* msg)
{
    Companion::get(this).processCDRMsg(source_locator, reception_locator, msg);
}

void MessageReceiver::associateEndpoint(
        Endpoint* to_add)
{
    Companion::get(this).associateEndpoint(to_add);
}

void MessageReceiver::removeEndpoint(
        Endpoint* to_remove)
{
    Companion::get(this).removeEndpoint(to_remove);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

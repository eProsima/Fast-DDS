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
 * @file MessageReceiver.h
 */



#ifndef FASTDDS_RTPS_MESSAGES__MESSAGERECEIVER_H
#define FASTDDS_RTPS_MESSAGES__MESSAGERECEIVER_H
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/common/Locator.hpp>

#include <rtps/writer/StatefulWriter.hpp>
#include <rtps/writer/StatelessWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSWriter;
class RTPSReader;
struct SubmessageHeader_t;
class ReceiverResource;

/**
 * Class MessageReceiver, process the received messages.
 * @ingroup MANAGEMENT_MODULE
 */
class MessageReceiver
{
public:

    MessageReceiver(
            RTPSParticipantImpl* /*participant*/,
            ReceiverResource* /*receiverResource*/)
    {

    }

    virtual ~MessageReceiver()
    {
    }

    void reset()
    {
    }

    void init(
            uint32_t /*rec_buffer_size*/)
    {
    }

    virtual void processCDRMsg(
            const Locator_t& /*loc*/,
            CDRMessage_t* /*msg*/)
    {
    }

    void setReceiverResource(
            ReceiverResource* /*receiverResource*/)
    {
    }

};
} // namespace rtps
} // namespace rtps
} // namespace eprosima
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif // FASTDDS_RTPS_MESSAGES__MESSAGERECEIVER_H

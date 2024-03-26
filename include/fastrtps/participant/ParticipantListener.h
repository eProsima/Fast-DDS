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
 * @file ParticipantListener.h
 *
 */

#ifndef __PARTICIPANT_PARTICIPANTLISTENER_H__
#define __PARTICIPANT_PARTICIPANTLISTENER_H__

#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastdds/rtps/reader/ReaderDiscoveryInfo.h>
#include <fastdds/rtps/writer/WriterDiscoveryInfo.h>

namespace eprosima {
namespace fastrtps {

class Participant;

/**
 * Class ParticipantListener, overrides behaviour towards certain events.
 * @ingroup FASTRTPS_MODULE
 */
class ParticipantListener
{
    public:

        ParticipantListener() {}

        virtual ~ParticipantListener() {}

        /*!
         * This method is called when a new Participant is discovered, or a previously discovered participant changes
         * its QOS or is removed.
         * @param participant Pointer to the Participant which discovered the remote participant.
         * @param info Remote participant information. User can take ownership of the object.
         */
        virtual void onParticipantDiscovery(Participant* participant, rtps::ParticipantDiscoveryInfo&& info)
        {
            (void)participant, (void)info;
        }

#if HAVE_SECURITY
        virtual void onParticipantAuthentication(Participant* participant, rtps::ParticipantAuthenticationInfo&& info)
        {
            (void)participant, (void)info;
        }
#endif

        /*!
         * This method is called when a new Subscriber is discovered, or a previously discovered subscriber changes
         * its QOS or is removed.
         * @param participant Pointer to the Participant which discovered the remote subscriber.
         * @param info Remote subscriber information. User can take ownership of the object.
         */
        virtual void onSubscriberDiscovery(Participant* participant, rtps::ReaderDiscoveryInfo&& info)
        {
            (void)participant, (void)info;
        }

        /*!
         * This method is called when a new Publisher is discovered, or a previously discovered publisher changes
         * its QOS or is removed.
         * @param participant Pointer to the Participant which discovered the remote publisher.
         * @param info Remote publisher information. User can take ownership of the object.
         */
        virtual void onPublisherDiscovery(Participant* participant, rtps::WriterDiscoveryInfo&& info)
        {
            (void)participant, (void)info;
        }
};

} // namespace fastrtps
} // namespace eprosima

#endif // __PARTICIPANT_PARTICIPANTLISTENER_H__

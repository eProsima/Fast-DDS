// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DomainParticipantListener.hpp
 *
 */

#ifndef __FASTDDS__PARTICIPANT_PARTICIPANTLISTENER_H__
#define __FASTDDS__PARTICIPANT_PARTICIPANTLISTENER_H__

#include "../../fastrtps/rtps/participant/ParticipantDiscoveryInfo.h"
#include "../../fastrtps/rtps/reader/ReaderDiscoveryInfo.h"
#include "../../fastrtps/rtps/writer/WriterDiscoveryInfo.h"

namespace eprosima {
namespace fastdds {

class DomainParticipant;

/**
 * Class DomainParticipantListener, overrides behaviour towards certain events.
 * @ingroup FASTRTPS_MODULE
 */
class DomainParticipantListener
{
    public:

        DomainParticipantListener() {}

        virtual ~DomainParticipantListener() {}

        /*!
         * This method is called when a new Participant is discovered, or a previously discovered participant changes
         * its QOS or is removed.
         * @param participant Pointer to the Participant which discovered the remote participant.
         * @param info Remote participant information. User can take ownership of the object.
         */
        virtual void onParticipantDiscovery(
                DomainParticipant* participant,
                fastrtps::rtps::ParticipantDiscoveryInfo&& info)
        {
            (void)participant, (void)info;
        }

#if HAVE_SECURITY
        virtual void onParticipantAuthentication(
                DomainParticipant* participant,
                fastrtps::rtps::ParticipantAuthenticationInfo&& info)
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
        virtual void onSubscriberDiscovery(
                DomainParticipant* participant,
                fastrtps::rtps::ReaderDiscoveryInfo&& info)
        {
            (void)participant, (void)info;
        }

        /*!
         * This method is called when a new Publisher is discovered, or a previously discovered publisher changes
         * its QOS or is removed.
         * @param participant Pointer to the Participant which discovered the remote publisher.
         * @param info Remote publisher information. User can take ownership of the object.
         */
        virtual void onPublisherDiscovery(
                DomainParticipant* participant,
                fastrtps::rtps::WriterDiscoveryInfo&& info)
        {
            (void)participant, (void)info;
        }

        // TODO: Methods in DomainParticipantListener (p.33 - DDS)
};

} // namespace fastdds
} // namespace eprosima

#endif // __FASTDDS__PARTICIPANT_PARTICIPANTLISTENER_H__

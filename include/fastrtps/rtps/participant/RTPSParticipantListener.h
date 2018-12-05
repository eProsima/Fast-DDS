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
 * @file RTPSParticipantListener.h
 *
 */

#ifndef __RTPS_PARTICIPANT_RTPSPARTICIPANTLISTENER_H__
#define __RTPS_PARTICIPANT_RTPSPARTICIPANTLISTENER_H__

#include "ParticipantDiscoveryInfo.h"
#include "../reader/ReaderDiscoveryInfo.h"
#include "../writer/WriterDiscoveryInfo.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{

class RTPSParticipant;

/**
* Class RTPSParticipantListener with virtual method that the user can overload to respond to certain events.
* @ingroup RTPS_MODULE
*/
class RTPS_DllAPI RTPSParticipantListener
{
    public:

        RTPSParticipantListener(){};

        virtual ~RTPSParticipantListener(){};

        /*!
         * This method is called when a new Participant is discovered, or a previously discovered participant changes
         * its QOS or is removed.
         * @param participant Pointer to the Participant which discovered the remote participant.
         * @param info Remote participant information. User can take ownership of the object.
         */
        virtual void onParticipantDiscovery(RTPSParticipant* participant, ParticipantDiscoveryInfo&& info)
        {
            (void)participant, (void)info;
        }

#if HAVE_SECURITY
        virtual void onParticipantAuthentication(RTPSParticipant* participant, ParticipantAuthenticationInfo&& info)
        {
            (void)participant, (void)info;
        }
#endif

        /*!
         * This method is called when a new Reader is discovered, or a previously discovered reader changes
         * its QOS or is removed.
         * @param participant Pointer to the Participant which discovered the remote reader.
         * @param info Remote reader information. User can take ownership of the object.
         */
        virtual void onReaderDiscovery(RTPSParticipant* participant, ReaderDiscoveryInfo&& info)
        {
            (void)participant, (void)info;
        }

        /*!
         * This method is called when a new Writer is discovered, or a previously discovered writer changes
         * its QOS or is removed.
         * @param participant Pointer to the Participant which discovered the remote writer.
         * @param info Remote writer information. User can take ownership of the object.
         */
        virtual void onWriterDiscovery(RTPSParticipant* participant, WriterDiscoveryInfo&& info)
        {
            (void)participant, (void)info;
        }
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif //__RTPS_PARTICIPANT_RTPSPARTICIPANTLISTENER_H__

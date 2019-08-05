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

#ifndef __FASTDDS__PARTICIPANT_PARTICIPANTLISTENER_HPP__
#define __FASTDDS__PARTICIPANT_PARTICIPANTLISTENER_HPP__

#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastdds/rtps/reader/ReaderDiscoveryInfo.h>
#include <fastdds/rtps/writer/WriterDiscoveryInfo.h>

#include <fastrtps/types/TypeIdentifier.h>
#include <fastrtps/types/TypeObject.h>
#include <fastrtps/types/DynamicTypePtr.h>

namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipant;

/**
 * Class DomainParticipantListener, overrides behaviour towards certain events.
 * @ingroup FASTDDS_MODULE
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
    virtual void on_participant_discovery(
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
    virtual void on_subscriber_discovery(
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
    virtual void on_publisher_discovery(
            DomainParticipant* participant,
            fastrtps::rtps::WriterDiscoveryInfo&& info)
    {
        (void)participant, (void)info;
    }

    /*!
     * This method is called when a participant discovers a new Type
     * The ownership of all object belongs to the caller so if needs to be used after the
     * method ends, a full copy should be perform (except for dyn_type due to its shared_ptr nature.
     * For example:
     * fastrtps::types::TypeIdentifier new_type_id = *identifier;
     */
    virtual void on_type_discovery(
            DomainParticipant* participant,
            const fastrtps::string_255& topic,
            const fastrtps::types::TypeIdentifier* identifier,
            const fastrtps::types::TypeObject* object,
            fastrtps::types::DynamicType_ptr dyn_type)
    {
        (void)participant, (void)topic, (void)identifier, (void)object, (void)dyn_type;
    }

    // TODO: Methods in DomainParticipantListener (p.33 - DDS)
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // __FASTDDS__PARTICIPANT_PARTICIPANTLISTENER_HPP__

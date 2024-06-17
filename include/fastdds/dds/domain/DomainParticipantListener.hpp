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

#include <fastdds/dds/publisher/PublisherListener.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>
#include <fastdds/dds/topic/TopicListener.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastdds/rtps/reader/ReaderDiscoveryInfo.h>
#include <fastdds/rtps/writer/WriterDiscoveryInfo.h>

namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipant;

/**
 * Class DomainParticipantListener, overrides behaviour towards certain events.
 *
 * @ingroup FASTDDS_MODULE
 */
class DomainParticipantListener :
    public PublisherListener,
    public SubscriberListener,
    public TopicListener
{
public:

    /**
     * @brief Constructor
     */
    DomainParticipantListener()
    {
    }

    /**
     * @brief Destructor
     */
    virtual ~DomainParticipantListener()
    {
    }

    /*!
     * This method is called when a new Participant is discovered, or a previously discovered participant changes
     * its QOS or is removed.
     *
     * @param[out] participant Pointer to the Participant which discovered the remote participant.
     * @param[out] info Remote participant information. User can take ownership of the object.
     * @param[out] should_be_ignored Flag to indicate the library to automatically ignore the discovered Participant.
     */
    virtual void on_participant_discovery(
            DomainParticipant* participant,
            rtps::ParticipantDiscoveryInfo&& info,
            bool& should_be_ignored)
    {
        static_cast<void>(participant);
        static_cast<void>(info);
        should_be_ignored = false;
    }

#if HAVE_SECURITY
    /*!
     * This method is called when a new Participant is authenticated.
     *
     * @param[out] participant Pointer to the authenticated Participant.
     * @param[out] info Remote participant authentication information. User can take ownership of the object.
     */
    virtual void onParticipantAuthentication(
            DomainParticipant* participant,
            rtps::ParticipantAuthenticationInfo&& info)
    {
        static_cast<void>(participant);
        static_cast<void>(info);
    }

#endif // if HAVE_SECURITY

    /*!
     * This method is called when a new DataReader is discovered, or a previously discovered DataReader changes
     * its QOS or is removed.
     *
     * @param[out] participant Pointer to the Participant which discovered the remote DataReader.
     * @param[out] info Remote DataReader information. User can take ownership of the object.
     * @param[out] should_be_ignored Flag to indicate the library to automatically ignore the discovered DataReader.
     */
    virtual void on_data_reader_discovery(
            DomainParticipant* participant,
            rtps::ReaderDiscoveryInfo&& info,
            bool& should_be_ignored)
    {
        static_cast<void>(participant);
        static_cast<void>(info);
        static_cast<void>(should_be_ignored);
    }

    /*!
     * This method is called when a new DataWriter is discovered, or a previously discovered DataWriter changes
     * its QOS or is removed.
     *
     * @param[out] participant Pointer to the Participant which discovered the remote DataWriter.
     * @param[out] info Remote DataWriter information. User can take ownership of the object.
     * @param[out] should_be_ignored Flag to indicate the library to automatically ignore the discovered DataWriter.
     */
    virtual void on_data_writer_discovery(
            DomainParticipant* participant,
            rtps::WriterDiscoveryInfo&& info,
            bool& should_be_ignored)
    {
        static_cast<void>(participant);
        static_cast<void>(info);
        static_cast<void>(should_be_ignored);
    }

    // TODO: Methods in DomainParticipantListener (p.33 - DDS)
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // __FASTDDS__PARTICIPANT_PARTICIPANTLISTENER_HPP__

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
#include <fastdds/dds/publisher/PublisherListener.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>
#include <fastdds/dds/topic/TopicListener.hpp>

#include <fastrtps/types/TypeIdentifier.h>
#include <fastrtps/types/TypeObject.h>
#include <fastrtps/types/DynamicTypePtr.h>

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
     */
    FASTDDS_TODO_BEFORE(3, 0, "Remove this overload")
    virtual void on_participant_discovery(
            DomainParticipant* participant,
            fastrtps::rtps::ParticipantDiscoveryInfo&& info)
    {
        static_cast<void>(participant);
        static_cast<void>(info);
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
            fastrtps::rtps::ParticipantDiscoveryInfo&& info,
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
            fastrtps::rtps::ParticipantAuthenticationInfo&& info)
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
     */
    virtual void on_subscriber_discovery(
            DomainParticipant* participant,
            fastrtps::rtps::ReaderDiscoveryInfo&& info)
    {
        static_cast<void>(participant);
        static_cast<void>(info);
    }

    /*!
     * This method is called when a new DataReader is discovered, or a previously discovered DataReader changes
     * its QOS or is removed.
     *
     * @warning Not Supported. This callback will never be called in the current version.
     *
     * @param[out] participant Pointer to the Participant which discovered the remote DataReader.
     * @param[out] info Remote DataReader information. User can take ownership of the object.
     * @param[out] should_be_ignored Flag to indicate the library to automatically ignore the discovered DataReader.
     */
    virtual void on_subscriber_discovery(
            DomainParticipant* participant,
            fastrtps::rtps::ReaderDiscoveryInfo&& info,
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
     */
    virtual void on_publisher_discovery(
            DomainParticipant* participant,
            fastrtps::rtps::WriterDiscoveryInfo&& info)
    {
        static_cast<void>(participant);
        static_cast<void>(info);
    }

    /*!
     * This method is called when a new DataWriter is discovered, or a previously discovered DataWriter changes
     * its QOS or is removed.
     *
     * @warning Not Supported. This callback will never be called in the current version.
     *
     * @param[out] participant Pointer to the Participant which discovered the remote DataWriter.
     * @param[out] info Remote DataWriter information. User can take ownership of the object.
     * @param[out] should_be_ignored Flag to indicate the library to automatically ignore the discovered DataWriter.
     */
    virtual void on_publisher_discovery(
            DomainParticipant* participant,
            fastrtps::rtps::WriterDiscoveryInfo&& info,
            bool& should_be_ignored)
    {
        static_cast<void>(participant);
        static_cast<void>(info);
        static_cast<void>(should_be_ignored);
    }

    /*!
     * This method is called when a participant discovers a new Type
     * The ownership of all object belongs to the caller so if needs to be used after the
     * method ends, a full copy should be perform (except for dyn_type due to its shared_ptr nature.
     * For example:
     * fastrtps::types::TypeIdentifier new_type_id = \*identifier;
     */
    virtual void on_type_discovery(
            DomainParticipant* participant,
            const fastrtps::rtps::SampleIdentity& request_sample_id,
            const fastrtps::string_255& topic,
            const fastrtps::types::TypeIdentifier* identifier,
            const fastrtps::types::TypeObject* object,
            fastrtps::types::DynamicType_ptr dyn_type)
    {
        static_cast<void>(participant);
        static_cast<void>(request_sample_id);
        static_cast<void>(topic);
        static_cast<void>(identifier);
        static_cast<void>(object);
        static_cast<void>(dyn_type);
    }

    /*!
     * This method is called when the typelookup client received a reply to a getTypeDependencies request.
     * The user may want to retrieve these new types using the getTypes request and create a new
     * DynamicType using the retrieved TypeObject.
     */
    virtual void on_type_dependencies_reply(
            DomainParticipant* participant,
            const fastrtps::rtps::SampleIdentity& request_sample_id,
            const fastrtps::types::TypeIdentifierWithSizeSeq& dependencies)
    {
        static_cast<void>(participant);
        static_cast<void>(request_sample_id);
        static_cast<void>(dependencies);
    }

    /*!
     * This method is called when a participant receives a TypeInformation while discovering another participant.
     */
    virtual void on_type_information_received(
            DomainParticipant* participant,
            const fastrtps::string_255 topic_name,
            const fastrtps::string_255 type_name,
            const fastrtps::types::TypeInformation& type_information)
    {
        static_cast<void>(participant);
        static_cast<void>(topic_name);
        static_cast<void>(type_name);
        static_cast<void>(type_information);
    }

    // TODO: Methods in DomainParticipantListener (p.33 - DDS)
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // __FASTDDS__PARTICIPANT_PARTICIPANTLISTENER_HPP__

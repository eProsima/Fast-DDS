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

#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/dds/core/status/DeadlineMissedStatus.hpp>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastdds/dds/core/status/SampleRejectedStatus.hpp>
#include <fastdds/dds/core/status/LivelinessChangedStatus.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipant;
class Subscriber;
class Publisher;
class Topic;
class DataWriter;
class DataReader;

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
            const fastrtps::rtps::SampleIdentity& request_sample_id,
            const fastrtps::string_255& topic,
            const fastrtps::types::TypeIdentifier* identifier,
            const fastrtps::types::TypeObject* object,
            fastrtps::types::DynamicType_ptr dyn_type)
    {
        (void)participant, (void)request_sample_id, (void)topic, (void)identifier, (void)object, (void)dyn_type;
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
        (void)participant, (void)request_sample_id, (void)dependencies;
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
        (void)participant, (void)topic_name, (void)type_name, (void)type_information;
    }

    /*!
     * This method is called when there are two topics with the same name but different characteristics.
     * This callback is notified of the InconsistentTopicStatus changes, if those changes are not captured by the TopicListener.
     */
    virtual void on_inconsistent_topic(
            Topic* topic,
            InconsistentTopicStatus status)
    {
        (void) topic, (void) status;
    }

    /*!
     * This method is called when the liveliness that the DataWriter has commited was not respected and due to this the DataReader
     * thinks that the DataWriter is no longer alive.
     * This callback is notified of the LivelinessLostStatus changes, if those changes are not captured by the DataWriterListener.
     */
    virtual void on_liveliness_lost(
            DataWriter* writer,
            LivelinessLostStatus status)
    {
        (void) writer, (void) status;
    }

    /*!
     * This method is called when the deadline commited by the DataWriter was not respected for a specific instance.
     * This callback is notified of the OfferedDeadlineMissedStatus changes, if those change are not captured by the DataWriterListener.
     */
    virtual void on_offered_deadline_missed(
            DataWriter* writer,
            OfferedDeadlineMissedStatus status)
    {
        (void) writer, (void) status;
    }

    /*!
     * This method is called when the QoS offered by the DataWriter is not compatible with the one requested by the DataReader.
     * This callback is notified of the OfferedIncompatibleQosStatus changes, if those changes are not captured by the DataWriterListener.
     */
    virtual void on_offered_incompatible_qos(
            DataWriter* writer,
            OfferedIncompatibleQosStatus status)
    {
        (void) writer, (void) status;
    }

    /*!
     * This method is called when there is new information available on the Subscriber side.
     * This callback is notified if the status change is not captured by the SubscriberListener.
     */
    virtual void on_data_on_readers(
            Subscriber* subscriber)
    {
        (void) subscriber;
    }

    /*!
     * This method is called when a sample has been lost.
     * This callback is notified of the SampleLostStatus changes, if those changes are not captured by the DataReaderListener.
     */
    virtual void on_sample_lost(
            DataReader* reader,
            SampleLostStatus status)
    {
        (void) reader, (void) status;
    }

    /*!
     * This method is called when there is new information available on the DataReader side.
     * This callback is notified if the status change is not captured by the DataReaderListener.
     */
    virtual void on_data_available(
            DataReader* reader)
    {
        (void) reader;
    }

    /*!
     * This method is called when a sample has been received but rejected.
     * This callback is notified of hte SampleRejectedStatus changes, if those changes are not captured by the DataReaderListener.
     */
    virtual void on_sample_rejected(
            DataReader* reader,
            SampleRejectedStatus status)
    {
        (void) reader, (void) status;
    }

    /*!
     * This method is called when the liveliness of one or more DataWriters that were writing instances read through the DataReader
     * has changed.
     * This callback is notified of the LivelinessChangedStatus changes, if those changes are not captured by the DataReaderListener.
     */
    virtual void on_liveliness_changed(
            DataReader* reader,
            LivelinessChangedStatus status)
    {
        (void) reader, (void) status;
    }

    /*!
     * This method is called when the deadline that the DataReader was expecting was not respected for a specific instance.
     * This callback is notified of the RequestedDeadlineMissedStatus changes, if those changes are not captured by the DataReaderListener.
     */
    virtual void on_requested_deadline_missed(
            DataReader* reader,
            RequestedDeadlineMissedStatus status)
    {
        (void) reader, (void) status;
    }

    /*!
     * This method is called when the Qos requested by the DataReader is not compatible with the one offered by the DataWriter.
     * This callback is notified of the RequestedIncompatibleQosStatus changes, if those changes are not captured by the DataReaderListener.
     */
    virtual void on_requested_incompatible_qos(
            DataReader* reader,
            RequestedIncompatibleQosStatus status)

    {
        (void) reader, (void) status;
    }

    /*!
     * This method is called when the DataWriter found a compatible DataReader(same Topic and compatible Qos) or ceased the connection with a previously
     * matched DataReader.
     * This callback is notified of the PublicationMatchedStatus changes, if those changes are not captured by the DataWriterListener.
     */
    virtual void on_publication_matched(
            DataWriter* writer,
            PublicationMatchedStatus status)
    {
        (void) writer, (void) status;
    }

    /*!
     * This method is called when the DataReader found a compatible DataWriter(same Topic and compatible Qos) or ceased the connection with a previously
     * matched DataWriter.
     * This callback is notified of the SubscriptionMatchedStatus changes, if those changes are not captured by the DataReaderListener.
     */
    virtual void on_subscription_matched(
            DataReader* reader,
            SubscriptionMatchedStatus status)
    {
        (void) reader, (void) status;
    }

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // __FASTDDS__PARTICIPANT_PARTICIPANTLISTENER_HPP__

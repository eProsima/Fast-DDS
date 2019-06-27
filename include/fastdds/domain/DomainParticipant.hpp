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
 * @file DomainParticipant.hpp
 *
 */

#ifndef _FASTDDS_DOMAIN_PARTICIPANT_H_
#define _FASTDDS_DOMAIN_PARTICIPANT_H_

#include "../../fastrtps/rtps/common/Guid.h"
#include "../../fastrtps/rtps/attributes/RTPSParticipantAttributes.h"

#include <utility>

namespace eprosima{
namespace fastrtps{

namespace rtps{
class RTPSParticipant;
class WriterProxyData;
class ReaderProxyData;
class ResourceEvent;
}

class TopicDataType;
class ParticipantAttributes;
class PublisherAttributes;
class SubscriberAttributes;

} // namespace fastrtps

namespace fastdds {

class DomainParticipantImpl;
class DomainParticipantListener;
class Publisher;
class PublisherQos;
class PublisherImpl;
class PublisherListener;
class Subscriber;
class SubscriberQos;
class SubscriberImpl;
class SubscriberListener;

/**
 * Class DomainParticipant used to group Publishers and Subscribers into a single working unit.
 * @ingroup FASTRTPS_MODULE
 */
class RTPS_DllAPI DomainParticipant
{
public:

    bool set_listener(
            DomainParticipantListener* listener);

    const DomainParticipantListener* get_listener() const;

    /**
     * Create a Publisher in this Participant.
     * @param qos QoS of the Publisher.
     * @param att Attributes of the Publisher. TopicAttributes and WriterQos will be ignored using DDS interface.
     * @param listen Pointer to the listener.
     * @return Pointer to the created Publisher.
     */
    Publisher* create_publisher(
            const fastdds::PublisherQos& qos,
            const fastrtps::PublisherAttributes& att,
            PublisherListener* listen = nullptr);

    bool delete_publisher(
            Publisher* publisher);

    /**
     * Create a Subscriber in this Participant.
     * @param qos QoS of the Subscriber.
     * @param att Attributes of the Subscriber. TopicAttributes and ReaderQos will be ignored using DDS interface.
     * @param listen Pointer to the listener.
     * @return Pointer to the created Subscriber.
     */
    Subscriber* create_subscriber(
            const fastdds::SubscriberQos& qos,
            const fastrtps::SubscriberAttributes& att,
            SubscriberListener* listen = nullptr);

    bool delete_subscriber(
            Subscriber* subscriber);

    /**
     * Register a type in this participant.
     * @param type Pointer to the TopicDatType.
     * @return True if registered.
     */
    bool register_type(
            fastrtps::TopicDataType* type);

    /**
     * Unregister a type in this participant.
     * @param typeName Name of the type
     * @return True if unregistered.
     */
    bool unregister_type(
            const char* typeName);

    // TODO create/delete topic

    Subscriber* get_builtin_subscriber();

    bool ignore_participant(
            const fastrtps::rtps::InstanceHandle_t& handle);

    bool ignore_topic(
            const fastrtps::rtps::InstanceHandle_t& handle);

    bool ignore_publication(
            const fastrtps::rtps::InstanceHandle_t& handle);

    bool ignore_subscription(
            const fastrtps::rtps::InstanceHandle_t& handle);

    uint8_t get_domain_id() const;

    bool delete_contained_entities();

    bool assert_liveliness();

    bool set_default_publisher_qos(
            const fastdds::PublisherQos& qos);

    const fastdds::PublisherQos& get_default_publisher_qos() const;

    bool set_default_subscriber_qos(
            const fastdds::SubscriberQos& qos);

    const fastdds::SubscriberQos& get_default_subscriber_qos() const;

    // TODO Get/Set default Topic Qos

    bool get_discovered_participants(
            std::vector<fastrtps::rtps::InstanceHandle_t>& participant_handles) const;

    /* TODO
    bool get_discovered_participant_data(
            ParticipantBuiltinTopicData& participant_data,
            const fastrtps::rtps::InstanceHandle_t& participant_handle) const;
    */

    bool get_discovered_topics(
            std::vector<fastrtps::rtps::InstanceHandle_t>& topic_handles) const;

    /* TODO
    bool get_discovered_topic_data(
            TopicBuiltinTopicData& topic_data,
            const fastrtps::rtps::InstanceHandle_t& topic_handle) const;
    */

    bool contains_entity(
            const fastrtps::rtps::InstanceHandle_t& handle,
            bool recursive = true) const;

    bool get_current_time(
            fastrtps::Time_t& current_time) const;

    const fastrtps::rtps::RTPSParticipant* rtps_participant() const;

    fastrtps::rtps::RTPSParticipant* rtps_participant();

    fastrtps::TopicDataType* find_type(
            const std::string& type_name) const;

    const fastrtps::rtps::InstanceHandle_t& get_instance_handle() const;

    // From here legacy RTPS methods.

    const fastrtps::rtps::GUID_t& guid() const;

    /**
     * Get the participant attributes
     * @return Participant attributes
     */
    const fastrtps::ParticipantAttributes& get_attributes() const;

    std::vector<std::string> getParticipantNames() const;

    /**
     * This method can be used when using a StaticEndpointDiscovery mechanism differnet that the one
     * included in FastRTPS, for example when communicating with other implementations.
     * It indicates the Participant that an Endpoint from the XML has been discovered and
     * should be activated.
     * @param partguid Participant GUID_t.
     * @param userId User defined ID as shown in the XML file.
     * @param kind EndpointKind (WRITER or READER)
     * @return True if correctly found and activated.
     */
    bool newRemoteEndpointDiscovered(
        const fastrtps::rtps::GUID_t& partguid,
        uint16_t userId,
        fastrtps::rtps::EndpointKind_t kind);

    bool get_remote_writer_info(
        const fastrtps::rtps::GUID_t& writerGuid,
        fastrtps::rtps::WriterProxyData& returnedInfo);

    bool get_remote_reader_info(
        const fastrtps::rtps::GUID_t& readerGuid,
        fastrtps::rtps::ReaderProxyData& returnedInfo);

    fastrtps::rtps::ResourceEvent& get_resource_event() const;

private:

    DomainParticipant();

    virtual ~DomainParticipant();

    DomainParticipantImpl* impl_;

    friend class DomainParticipantFactory;

    friend class DomainParticipantImpl;
};

} // namespace fastdds
} /* namespace eprosima */

#endif /* _FASTDDS_DOMAIN_PARTICIPANT_H_ */

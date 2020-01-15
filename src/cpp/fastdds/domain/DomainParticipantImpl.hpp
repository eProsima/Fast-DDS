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
 * @file DomainParticipantImpl.h
 *
 */

#ifndef _FASTDDS_PARTICIPANTIMPL_HPP_
#define _FASTDDS_PARTICIPANTIMPL_HPP_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/participant/RTPSParticipantListener.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastdds/rtps/reader/StatefulReader.h>

#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>
#include <fastdds/dds/subscriber/BuiltinSubscriber.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include <dds/core/status/Status.hpp>

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastrtps/types/TypesBase.h>

#include <dds/core/status/State.hpp>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastrtps {

namespace rtps {

class RTPSParticipant;
class WriterProxyData;
class ReaderProxyData;

} //namespace rtps

class PublisherAttributes;
class SubscriberAttributes;

} // namespace fastrtps

namespace fastdds {
namespace dds {

class DomainParticipant;
class DomainParticipantListener;
class Publisher;
class PublisherImpl;
class PublisherListener;
class Subscriber;
class SubscriberImpl;
class SubscriberListener;
class TopicImpl;
class Topic;
class TopicListener;
class TopicDescription;

/**
 * This is the implementation class of the DomainParticipant.
 * @ingroup FASTRTPS_MODULE
 */
class DomainParticipantImpl
{
    friend class DomainParticipantFactory;

private:

    DomainParticipantImpl(
            const fastrtps::ParticipantAttributes& patt,
            DomainParticipant* pspart,
            const DomainParticipantQos& qos = PARTICIPANT_QOS_DEFAULT,
            DomainParticipantListener* listen = nullptr);

    virtual ~DomainParticipantImpl();

public:

    ReturnCode_t set_listener(
            DomainParticipantListener* listener)
    {
        listener_ = listener;
        return ReturnCode_t::RETCODE_OK;
    }

    const DomainParticipantListener* get_listener() const
    {
        return listener_;
    }

    DomainParticipantListener* get_listener()
    {
        return listener_;
    }

    /**
     * Create a Publisher in this Participant.
     * @param qos QoS of the Publisher.
     * @param att Attributes of the Publisher.
     * @param listen Pointer to the listener.
     * @return Pointer to the created Publisher.
     */
    Publisher* create_publisher(
            const fastdds::dds::PublisherQos& qos,
            const fastrtps::PublisherAttributes& att,
            PublisherListener* listen = nullptr,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

    ReturnCode_t delete_publisher(
            Publisher* publisher);

    /**
     * Create a Subscriber in this Participant.
     * @param qos QoS of the Subscriber.
     * @param att Attributes of the Subscriber
     * @param listen Pointer to the listener.
     * @return Pointer to the created Subscriber.
     */
    Subscriber* create_subscriber(
            const fastdds::dds::SubscriberQos& qos,
            const fastrtps::SubscriberAttributes& att,
            SubscriberListener* listen = nullptr,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

    ReturnCode_t delete_subscriber(
            Subscriber* subscriber);

    /**
     * Create a Topic in this Participant.
     * @param topic_name Name of the Topic.
     * @param type_name Name of the TypeSupport.
     * @param qos QoS of the Topic.
     * @param listen Pointer to the listener.
     * @param mask Mask containing the status enabled of the Topic.
     * @return Pointer to the created Topic.
     */
    Topic* create_topic(
            std::string topic_name,
            std::string type_name,
            const fastdds::dds::TopicQos& qos,
            TopicListener* listen = nullptr,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

    ReturnCode_t delete_topic(
            Topic* topic);

    Topic* find_topic(
            const std::string& topic_name,
            const Duration_t& timeout);

    TopicDescription* lookup_topicdescription(
            const std::string& topic_name);

    /**
     * Register a type in this participant.
     * @param type The TypeSupport to register. A copy will be kept by the participant until removed.
     * @param type_name The name that will be used to identify the Type.
     * @return True if registered.
     */
    ReturnCode_t register_type(
            TypeSupport type,
            const std::string& type_name);

    /**
     * @brief Registers into types Factories an already registered dynamic type
     * to ease its use through factories.
     * @param type_name
     * @return True if registered.
     */
    ReturnCode_t register_dynamic_type_to_factories(
            const std::string& type_name) const;

    /**
     * Unregister a type in this participant.
     * @param typeName Name of the type
     * @return True if unregistered.
     */
    ReturnCode_t unregister_type(
            const char* typeName);

    BuiltinSubscriber* get_builtin_subscriber();

    /* TODO
       bool ignore_participant(
            const fastrtps::rtps::InstanceHandle_t& handle);
     */

    /* TODO
       bool ignore_topic(
            const fastrtps::rtps::InstanceHandle_t& handle);
     */

    /* TODO
       bool ignore_publication(
            const fastrtps::rtps::InstanceHandle_t& handle);
     */

    /* TODO
       bool ignore_subscription(
            const fastrtps::rtps::InstanceHandle_t& handle);
     */

    uint8_t get_domain_id() const;

    ReturnCode_t delete_contained_entities();

    ReturnCode_t assert_liveliness();

    ReturnCode_t set_default_publisher_qos(
            const fastdds::dds::PublisherQos& qos);

    const fastdds::dds::PublisherQos& get_default_publisher_qos() const;

    ReturnCode_t set_default_subscriber_qos(
            const fastdds::dds::SubscriberQos& qos);

    const fastdds::dds::SubscriberQos& get_default_subscriber_qos() const;

    ReturnCode_t get_discovered_participants(
            std::vector<fastrtps::rtps::InstanceHandle_t>& participant_handles) const;

    ReturnCode_t get_discovered_participant_data(
            ParticipantBuiltinTopicData& participant_data,
            const fastrtps::rtps::InstanceHandle_t& participant_handle) const;

    ReturnCode_t get_discovered_topics(
            std::vector<fastrtps::rtps::InstanceHandle_t>& topic_handles) const;

    ReturnCode_t get_discovered_topic_data(
            TopicBuiltinTopicData& topic_data,
            const fastrtps::rtps::InstanceHandle_t& topic_handle) const;

    bool contains_entity(
            const fastrtps::rtps::InstanceHandle_t& handle,
            bool recursive = true) const;

    ReturnCode_t get_current_time(
            fastrtps::Time_t& current_time) const;

    const DomainParticipant* get_participant() const;

    DomainParticipant* get_participant();

    const fastrtps::rtps::RTPSParticipant* rtps_participant() const
    {
        return rtps_participant_;
    }

    fastrtps::rtps::RTPSParticipant* rtps_participant()
    {
        return rtps_participant_;
    }

    const TypeSupport find_type(
            const std::string& type_name) const;

    // From here legacy RTPS methods.

    const fastrtps::rtps::GUID_t& guid() const;

    /**
     * Get the participant attributes
     * @return Participant attributes
     */
    inline const fastrtps::ParticipantAttributes& get_attributes() const
    {
        return att_;
    }

    std::vector<std::string> get_participant_names() const;

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
    bool new_remote_endpoint_discovered(
            const fastrtps::rtps::GUID_t& partguid,
            uint16_t userId,
            fastrtps::rtps::EndpointKind_t kind);

    fastrtps::rtps::ResourceEvent& get_resource_event() const;

    fastrtps::rtps::SampleIdentity get_type_dependencies(
            const fastrtps::types::TypeIdentifierSeq& in) const;

    fastrtps::rtps::SampleIdentity get_types(
            const fastrtps::types::TypeIdentifierSeq& in) const;

    ReturnCode_t register_remote_type(
            const fastrtps::types::TypeInformation& type_information,
            const std::string& type_name,
            std::function<void(const std::string& name, const fastrtps::types::DynamicType_ptr type)>& callback);

    //! Remove all listeners in the hierarchy to allow a quiet destruction
    void disable();

    ReturnCode_t get_qos(
            DomainParticipantQos& qos) const;

    const DomainParticipantQos& get_qos() const
    {
        return qos_;
    }

    ReturnCode_t set_qos(
            const DomainParticipantQos& qos);

    ReturnCode_t set_default_topic_qos(
            const fastdds::dds::TopicQos& qos)
    {
        topic_qos_ = qos;
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t get_default_topic_qos(
            fastdds::dds::TopicQos& qos) const
    {
        qos = topic_qos_;
        return ReturnCode_t::RETCODE_OK;
    }

    const fastdds::dds::TopicQos& get_default_topic_qos() const
    {
        return topic_qos_;
    }

    ReturnCode_t autoenable_entities();

private:

    //!Participant Attributes
    fastrtps::ParticipantAttributes att_;

    //!Participant Qos
    DomainParticipantQos qos_;

    //!Topic Qos
    TopicQos topic_qos_;

    //!RTPSParticipant
    fastrtps::rtps::RTPSParticipant* rtps_participant_;

    //!Participant*
    DomainParticipant* participant_;

    //!Participant Listener
    DomainParticipantListener* listener_;

    //!Publisher maps
    std::map<Publisher*, PublisherImpl*> publishers_;
    std::map<fastrtps::rtps::InstanceHandle_t, Publisher*> publishers_by_handle_;
    mutable std::mutex mtx_pubs_;

    PublisherQos default_pub_qos_;

    //!Subscriber maps
    std::map<Subscriber*, SubscriberImpl*> subscribers_;
    std::map<fastrtps::rtps::InstanceHandle_t, Subscriber*> subscribers_by_handle_;
    mutable std::mutex mtx_subs_;

    SubscriberQos default_sub_qos_;

    //!TopicDataType map
    std::map<std::string, TypeSupport> types_;
    std::map<Topic*, std::pair<TopicImpl*, TopicDescription*> > topics_;
    std::map<std::string, Topic*> topics_by_name_; //Indexed by topic name
    std::map<fastrtps::rtps::InstanceHandle_t, Topic*> topics_by_handle_;
    mutable std::mutex mtx_types_;
    std::condition_variable cv_topic_;



    // Mutex for requests and callbacks maps.
    std::mutex mtx_request_cb_;

    // register_remote_type parent request, type_name, callback relationship.
    std::map<fastrtps::rtps::SampleIdentity,
            std::pair<std::string, std::function<void(
                const std::string& name,
                const fastrtps::types::DynamicType_ptr)> > > register_callbacks_;

    // Relationship between child and parent request
    std::map<fastrtps::rtps::SampleIdentity, fastrtps::rtps::SampleIdentity> child_requests_;

    // All parent's child requests
    std::map<fastrtps::rtps::SampleIdentity, std::vector<fastrtps::rtps::SampleIdentity> > parent_requests_;

    std::vector<fastrtps::rtps::InstanceHandle_t> discovered_participants_;

    std::vector<fastrtps::rtps::InstanceHandle_t> discovered_topics_;

    class MyRTPSParticipantListener : public fastrtps::rtps::RTPSParticipantListener
    {
public:

        MyRTPSParticipantListener(
                DomainParticipantImpl* impl)
            : participant_(impl)
        {
        }

        virtual ~MyRTPSParticipantListener() override
        {
        }

        void onParticipantDiscovery(
                fastrtps::rtps::RTPSParticipant* participant,
                fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;

#if HAVE_SECURITY
        void onParticipantAuthentication(
                fastrtps::rtps::RTPSParticipant* participant,
                fastrtps::rtps::ParticipantAuthenticationInfo&& info) override;
#endif

        void onReaderDiscovery(
                fastrtps::rtps::RTPSParticipant* participant,
                fastrtps::rtps::ReaderDiscoveryInfo&& info) override;

        void onWriterDiscovery(
                fastrtps::rtps::RTPSParticipant* participant,
                fastrtps::rtps::WriterDiscoveryInfo&& info) override;

        void on_type_discovery(
                fastrtps::rtps::RTPSParticipant* participant,
                const fastrtps::rtps::SampleIdentity& request_sample_id,
                const fastrtps::string_255& topic,
                const fastrtps::types::TypeIdentifier* identifier,
                const fastrtps::types::TypeObject* object,
                fastrtps::types::DynamicType_ptr dyn_type) override;

        void on_type_dependencies_reply(
                fastrtps::rtps::RTPSParticipant* participant,
                const fastrtps::rtps::SampleIdentity& request_sample_id,
                const fastrtps::types::TypeIdentifierWithSizeSeq& dependencies) override;

        void on_type_information_received(
                fastrtps::rtps::RTPSParticipant* participant,
                const fastrtps::string_255& topic_name,
                const fastrtps::string_255& type_name,
                const fastrtps::types::TypeInformation& type_information) override;

        DomainParticipantImpl* participant_;

    } rtps_listener_;

    bool exists_entity_id(
            const fastrtps::rtps::EntityId_t& entity_id) const;

    ReturnCode_t register_dynamic_type(
            fastrtps::types::DynamicType_ptr dyn_type);

    bool check_get_type_request(
            const fastrtps::rtps::SampleIdentity& requestId,
            const fastrtps::types::TypeIdentifier* identifier,
            const fastrtps::types::TypeObject* object,
            fastrtps::types::DynamicType_ptr dyn_type);

    bool check_get_dependencies_request(
            const fastrtps::rtps::SampleIdentity& requestId,
            const fastrtps::types::TypeIdentifierWithSizeSeq& dependencies);

    // Always call it with the mutex already taken
    void remove_parent_request(
            const fastrtps::rtps::SampleIdentity& request);

    // Always call it with the mutex already taken
    void remove_child_request(
            const fastrtps::rtps::SampleIdentity& request);

    // Always call it with the mutex already taken
    void on_child_requests_finished(
            const fastrtps::rtps::SampleIdentity& parent);

    void fill_pending_dependencies(
            const fastrtps::types::TypeIdentifierWithSizeSeq& dependencies,
            fastrtps::types::TypeIdentifierSeq& pending_identifiers,
            fastrtps::types::TypeIdentifierSeq& pending_objects) const;

    std::string get_inner_type_name(
            const fastrtps::rtps::SampleIdentity& id) const;

public:

    class Listener : public DomainParticipantListener
    {
public:

        Listener(
                DomainParticipantImpl* dp)
            : domain_participant_(dp)
        {
        }

        ~Listener() override
        {
        }

        void on_data_on_readers(
                Subscriber* subscriber) override;

        DomainParticipantImpl* domain_participant_;
    } subscriber_listener_;
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif
#endif /* _FASTDDS_PARTICIPANTIMPL_HPP_ */

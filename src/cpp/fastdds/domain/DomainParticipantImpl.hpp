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

#include <atomic>
#include <mutex>
#include <condition_variable>

#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/participant/RTPSParticipantListener.h>
#include <fastdds/rtps/reader/StatefulReader.h>

#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/ContentFilteredTopic.hpp>
#include <fastdds/dds/topic/IContentFilterFactory.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastrtps/types/TypesBase.h>

#include "fastdds/topic/DDSSQLFilter/DDSFilterFactory.hpp"
#include <fastdds/topic/TopicProxyFactory.hpp>

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
class ReaderFilterCollection;

/**
 * This is the implementation class of the DomainParticipant.
 * @ingroup FASTRTPS_MODULE
 */
class DomainParticipantImpl
{
    friend class DomainParticipantFactory;
    friend class DomainParticipant;
    friend class ReaderFilterCollection;

protected:

    DomainParticipantImpl(
            DomainParticipant* dp,
            DomainId_t did,
            const DomainParticipantQos& qos,
            DomainParticipantListener* listen = nullptr);

    virtual ~DomainParticipantImpl();

public:

    virtual ReturnCode_t enable();

    ReturnCode_t get_qos(
            DomainParticipantQos& qos) const;

    const DomainParticipantQos& get_qos() const;

    ReturnCode_t set_qos(
            const DomainParticipantQos& qos);

    ReturnCode_t set_listener(
            DomainParticipantListener* listener,
            const std::chrono::seconds timeout = std::chrono::seconds::max())
    {
        auto time_out = std::chrono::time_point<std::chrono::system_clock>::max();
        if (timeout < std::chrono::seconds::max())
        {
            auto now = std::chrono::system_clock::now();
            time_out = now + timeout;
        }

        std::unique_lock<std::mutex> lock(mtx_gs_);
        if (!cv_gs_.wait_until(lock, time_out, [this]
                {
                    // Proceed if no callbacks are being executed
                    return !(rtps_listener_.callback_counter_ > 0);
                }))
        {
            return ReturnCode_t::RETCODE_ERROR;
        }

        rtps_listener_.callback_counter_ = (listener == nullptr) ? -1 : 0;
        listener_ = listener;
        return ReturnCode_t::RETCODE_OK;
    }

    DomainParticipantListener* get_listener() const
    {
        std::lock_guard<std::mutex> _(mtx_gs_);
        return listener_;
    }

    /**
     * Create a Publisher in this Participant.
     * @param qos QoS of the Publisher.
     * @param listenerer Pointer to the listener.
     * @param mask StatusMask
     * @return Pointer to the created Publisher.
     */
    Publisher* create_publisher(
            const PublisherQos& qos,
            PublisherListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Create a Publisher in this Participant.
     * @param qos QoS of the Publisher.
     * @param[out] impl Return a pointer to the created Publisher's implementation.
     * @param listenerer Pointer to the listener.
     * @param mask StatusMask
     * @return Pointer to the created Publisher.
     */
    Publisher* create_publisher(
            const PublisherQos& qos,
            PublisherImpl** impl,
            PublisherListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Create a Publisher in this Participant.
     * @param profile_name Publisher profile name.
     * @param listener Pointer to the listener.
     * @param mask StatusMask
     * @return Pointer to the created Publisher.
     */
    Publisher* create_publisher_with_profile(
            const std::string& profile_name,
            PublisherListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    ReturnCode_t delete_publisher(
            const Publisher* publisher);

    /**
     * Create a Subscriber in this Participant.
     * @param qos QoS of the Subscriber.
     * @param listener Pointer to the listener.
     * @param mask StatusMask that holds statuses the listener responds to
     * @return Pointer to the created Subscriber.
     */
    Subscriber* create_subscriber(
            const SubscriberQos& qos,
            SubscriberListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Create a Subscriber in this Participant.
     * @param profile Subscriber profile name.
     * @param listener Pointer to the listener.
     * @param mask StatusMask
     * @return Pointer to the created Subscriber.
     */
    Subscriber* create_subscriber_with_profile(
            const std::string& profile_name,
            SubscriberListener* listener,
            const StatusMask& mask);

    ReturnCode_t delete_subscriber(
            const Subscriber* subscriber);

    /**
     * Create a Topic in this Participant.
     * @param topic_name Name of the Topic.
     * @param type_name Data type of the Topic.
     * @param qos QoS of the Topic.
     * @param listener Pointer to the listener.
     * @param mask StatusMask that holds statuses the listener responds to
     * @return Pointer to the created Topic.
     */
    Topic* create_topic(
            const std::string& topic_name,
            const std::string& type_name,
            const TopicQos& qos = TOPIC_QOS_DEFAULT,
            TopicListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Create a Topic in this Participant.
     * @param topic_name Name of the Topic.
     * @param type_name Data type of the Topic.
     * @param profile Topic profile name.
     * @param listener Pointer to the listener.
     * @param mask StatusMask that holds statuses the listener responds to
     * @return Pointer to the created Topic.
     */
    Topic* create_topic_with_profile(
            const std::string& topic_name,
            const std::string& type_name,
            const std::string& profile_name,
            TopicListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Gives access to an existing (or ready to exist) enabled Topic.
     * It should be noted that the returned Topic is a local object that acts as a proxy to designate the global
     * concept of topic.
     * Topics obtained by means of find_topic, must also be deleted by means of delete_topic so that the local
     * resources can be released.
     * If a Topic is obtained multiple times by means of find_topic or create_topic, it must also be deleted that same
     * number of times using delete_topic.
     *
     * @param topic_name Topic name
     * @param timeout Maximum time to wait for the Topic
     * @return Pointer to the existing Topic, nullptr in case of error or timeout
     */
    Topic* find_topic(
            const std::string& topic_name,
            const fastrtps::Duration_t& timeout);

    /**
     * Implementation of Topic::set_listener that propagates the listener and mask to all the TopicProxy
     * objects held by the same TopicProxy factory in a thread-safe way.
     *
     * @param factory  TopicProxyFactory managing the topic on which the listener should be changed.
     * @param listener Listener to assign to all the TopicProxy objects owned by the factory.
     * @param mask     StatusMask to assign to all the TopicProxy objects owned by the factory.
     */
    void set_topic_listener(
            const TopicProxyFactory* factory,
            TopicImpl* topic,
            TopicListener* listener,
            const StatusMask& mask);

    ReturnCode_t delete_topic(
            const Topic* topic);

    ContentFilteredTopic* create_contentfilteredtopic(
            const std::string& name,
            Topic* related_topic,
            const std::string& filter_expression,
            const std::vector<std::string>& expression_parameters,
            const char* filter_class_name);

    ReturnCode_t delete_contentfilteredtopic(
            const ContentFilteredTopic* topic);

    ReturnCode_t register_content_filter_factory(
            const char* filter_class_name,
            IContentFilterFactory* const filter_factory);

    IContentFilterFactory* lookup_content_filter_factory(
            const char* filter_class_name);

    ReturnCode_t unregister_content_filter_factory(
            const char* filter_class_name);

    /**
     * Looks up an existing, locally created @ref TopicDescription, based on its name.
     * May be called on a disabled participant.
     *
     * @param topic_name Name of the @ref TopicDescription to search for.
     *
     * @return Pointer to the topic description, if it has been created locally. Otherwhise, nullptr is returned.
     *
     * @remark UNSAFE. It is unsafe to lookup a topic description while another thread is creating a topic.
     */
    TopicDescription* lookup_topicdescription(
            const std::string& topic_name) const;

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
     * Unregister a type in this participant.
     * @param typeName Name of the type
     * @return True if unregistered.
     */
    ReturnCode_t unregister_type(
            const std::string& typeName);

    // TODO create/delete topic

    // TODO Subscriber* get_builtin_subscriber();

    /**
     * @brief Locally ignore a remote domain participant.
     *
     * @param[in] handle Identifier of the remote participant to ignore.
     * @return RETCODE_NOT_ENABLED if the participant is not enabled.
     *         RETCODE_ERROR if unable to ignore.
     *         RETCODE_OK if successful.
     *
     */
    ReturnCode_t ignore_participant(
            const InstanceHandle_t& handle);

    /* TODO
       bool ignore_topic(
            const InstanceHandle_t& handle);
     */

    /**
     * @brief Locally ignore a remote datawriter.
     *
     * @param[in] handle Identifier of the remote datawriter to ignore.
     * @return true if correctly ignored. False otherwise.
     */
    bool ignore_publication(
            const InstanceHandle_t& handle);

    /**
     * @brief Locally ignore a remote datareader.
     *
     * @param[in] handle Identifier of the remote datareader to ignore.
     * @return true if correctly ignored. False otherwise.
     */
    bool ignore_subscription(
            const InstanceHandle_t& handle);

    DomainId_t get_domain_id() const;

    virtual ReturnCode_t delete_contained_entities();

    ReturnCode_t assert_liveliness();

    ReturnCode_t set_default_publisher_qos(
            const PublisherQos& qos);

    void reset_default_publisher_qos();

    const PublisherQos& get_default_publisher_qos() const;

    const ReturnCode_t get_publisher_qos_from_profile(
            const std::string& profile_name,
            PublisherQos& qos) const;

    ReturnCode_t set_default_subscriber_qos(
            const SubscriberQos& qos);

    void reset_default_subscriber_qos();

    const SubscriberQos& get_default_subscriber_qos() const;

    const ReturnCode_t get_subscriber_qos_from_profile(
            const std::string& profile_name,
            SubscriberQos& qos) const;

    ReturnCode_t set_default_topic_qos(
            const TopicQos& qos);

    void reset_default_topic_qos();

    const TopicQos& get_default_topic_qos() const;

    const ReturnCode_t get_topic_qos_from_profile(
            const std::string& profile_name,
            TopicQos& qos) const;

    /* TODO
       bool get_discovered_participants(
            std::vector<InstanceHandle_t>& participant_handles) const;
     */

    /* TODO
       bool get_discovered_participant_data(
            ParticipantBuiltinTopicData& participant_data,
            const InstanceHandle_t& participant_handle) const;
     */

    /* TODO
       bool get_discovered_topics(
            std::vector<InstanceHandle_t>& topic_handles) const;
     */

    /* TODO
       bool get_discovered_topic_data(
            TopicBuiltinTopicData& topic_data,
            const InstanceHandle_t& topic_handle) const;
     */

    bool contains_entity(
            const InstanceHandle_t& handle,
            bool recursive = true) const;

    ReturnCode_t get_current_time(
            fastrtps::Time_t& current_time) const;

    const DomainParticipant* get_participant() const
    {
        std::lock_guard<std::mutex> _(mtx_gs_);
        return participant_;
    }

    DomainParticipant* get_participant()
    {
        std::lock_guard<std::mutex> _(mtx_gs_);
        return participant_;
    }

    const fastrtps::rtps::RTPSParticipant* get_rtps_participant() const
    {
        std::lock_guard<std::mutex> _(mtx_gs_);
        return rtps_participant_;
    }

    fastrtps::rtps::RTPSParticipant* get_rtps_participant()
    {
        std::lock_guard<std::mutex> _(mtx_gs_);
        return rtps_participant_;
    }

    const TypeSupport find_type(
            const std::string& type_name) const;

    const InstanceHandle_t& get_instance_handle() const;

    // From here legacy RTPS methods.

    const fastrtps::rtps::GUID_t& guid() const;

    std::vector<std::string> get_participant_names() const;

    /**
     * This method can be used when using a StaticEndpointDiscovery mechanism different that the one
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

    /**
     * Helps the user to solve all dependencies calling internally to the typelookup service and
     * registers the resulting dynamic type.
     * The registration may be perform asynchronously, case in which the user will be notified
     * through the given callback, which receives the type_name as unique argument.
     *
     * @param type_information
     * @param type_name
     * @param callback
     * @return RETCODE_OK If the given type_information is enough to build the type without using
     *         the typelookup service (callback will not be called).
     * @return RETCODE_OK if the given type is already available (callback will not be called).
     * @return RETCODE_NO_DATA if type is not available yet (the callback will be called if
     *         negotiation is success, and ignored in other case).
     * @return RETCODE_NOT_ENABLED if the DomainParticipant is not enabled.
     * @return RETCODE_PRECONDITION_NOT_MET if the DomainParticipant type lookup service is disabled.
     */
    ReturnCode_t register_remote_type(
            const fastrtps::types::TypeInformation& type_information,
            const std::string& type_name,
            std::function<void(const std::string& name, const fastrtps::types::DynamicType_ptr type)>& callback);

    //! Remove all listeners in the hierarchy to allow a quiet destruction
    virtual void disable();

    /**
     * This method checks if the DomainParticipant has created an entity that has not been
     * deleted.
     * @return true if the participant has no deleted entities, false otherwise
     */
    bool has_active_entities();

    /**
     * Returns the most appropriate listener to handle the callback for the given status,
     * or nullptr if there is no appropriate listener.
     */
    DomainParticipantListener* get_listener_for(
            const StatusMask& status);

    std::atomic<uint32_t>& id_counter()
    {
        return id_counter_;
    }

protected:

    //!Domain id
    DomainId_t domain_id_;

    //!Participant id
    int32_t participant_id_ = -1;

    //!Pre-calculated guid
    fastrtps::rtps::GUID_t guid_;

    //!For instance handle creation
    std::atomic<uint32_t> next_instance_id_;

    //!Participant Qos
    DomainParticipantQos qos_;

    //!RTPSParticipant
    fastrtps::rtps::RTPSParticipant* rtps_participant_;

    //!Participant*
    DomainParticipant* participant_;

    //!Participant Listener
    DomainParticipantListener* listener_;

    //! getter/setter mutex
    mutable std::mutex mtx_gs_;

    //! getter/setter condition variable
    std::condition_variable cv_gs_;

    //!Publisher maps
    std::map<Publisher*, PublisherImpl*> publishers_;
    std::map<InstanceHandle_t, Publisher*> publishers_by_handle_;
    mutable std::mutex mtx_pubs_;

    PublisherQos default_pub_qos_;

    //!Subscriber maps
    std::map<Subscriber*, SubscriberImpl*> subscribers_;
    std::map<InstanceHandle_t, Subscriber*> subscribers_by_handle_;
    mutable std::mutex mtx_subs_;

    SubscriberQos default_sub_qos_;

    //!TopicDataType map
    std::map<std::string, TypeSupport> types_;
    mutable std::mutex mtx_types_;

    //!Topic map
    std::map<std::string, TopicProxyFactory*> topics_;
    std::map<InstanceHandle_t, Topic*> topics_by_handle_;
    std::map<std::string, std::unique_ptr<ContentFilteredTopic>> filtered_topics_;
    std::map<std::string, IContentFilterFactory*> filter_factories_;
    DDSSQLFilter::DDSFilterFactory dds_sql_filter_factory_;
    mutable std::mutex mtx_topics_;
    std::condition_variable cond_topics_;

    TopicQos default_topic_qos_;

    // Mutex for requests and callbacks maps.
    std::mutex mtx_request_cb_;

    // register_remote_type parent request, type_name, callback relationship.
    std::map<fastrtps::rtps::SampleIdentity,
            std::pair<std::string, std::function<void(
                const std::string& name,
                const fastrtps::types::DynamicType_ptr)>>> register_callbacks_;

    // Relationship between child and parent request
    std::map<fastrtps::rtps::SampleIdentity, fastrtps::rtps::SampleIdentity> child_requests_;

    // All parent's child requests
    std::map<fastrtps::rtps::SampleIdentity, std::vector<fastrtps::rtps::SampleIdentity>> parent_requests_;

    std::atomic<uint32_t> id_counter_;

    class MyRTPSParticipantListener : public fastrtps::rtps::RTPSParticipantListener
    {
        struct Sentry
        {
            Sentry(
                    MyRTPSParticipantListener* listener)
                : listener_(listener)
                , on_guard_(false)
            {
                std::lock_guard<std::mutex> _(listener_->participant_->mtx_gs_);
                if (listener_ != nullptr && listener_->participant_ != nullptr &&
                        listener_->participant_->listener_ != nullptr &&
                        listener_->participant_->participant_ != nullptr)
                {
                    if (listener_->callback_counter_ >= 0)
                    {
                        ++listener_->callback_counter_;
                        on_guard_ = true;
                    }
                }
            }

            ~Sentry()
            {
                if (on_guard_)
                {
                    bool notify = false;
                    {
                        std::lock_guard<std::mutex> lock(listener_->participant_->mtx_gs_);
                        assert(
                            listener_ != nullptr && listener_->participant_ != nullptr && listener_->participant_->listener_ != nullptr &&
                            listener_->participant_->participant_ != nullptr);
                        --listener_->callback_counter_;
                        notify = !listener_->callback_counter_;
                    }
                    if (notify)
                    {
                        listener_->participant_->cv_gs_.notify_all();
                    }
                }
            }

            operator bool () const
            {
                return on_guard_;
            }

            MyRTPSParticipantListener* listener_ = nullptr;
            bool on_guard_;
        };

        using fastrtps::rtps::RTPSParticipantListener::onParticipantDiscovery;
        using fastrtps::rtps::RTPSParticipantListener::onReaderDiscovery;
        using fastrtps::rtps::RTPSParticipantListener::onWriterDiscovery;

    public:

        MyRTPSParticipantListener(
                DomainParticipantImpl* impl)
            : participant_(impl)
        {
        }

        virtual ~MyRTPSParticipantListener() override
        {
            assert(!(callback_counter_ > 0));
        }

        void onParticipantDiscovery(
                fastrtps::rtps::RTPSParticipant* participant,
                fastrtps::rtps::ParticipantDiscoveryInfo&& info,
                bool& should_be_ignored) override;

#if HAVE_SECURITY
        void onParticipantAuthentication(
                fastrtps::rtps::RTPSParticipant* participant,
                fastrtps::rtps::ParticipantAuthenticationInfo&& info) override;
#endif // if HAVE_SECURITY

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
        int callback_counter_ = 0;

    }
    rtps_listener_;

    void create_instance_handle(
            InstanceHandle_t& handle);

    ReturnCode_t register_dynamic_type(
            fastrtps::types::DynamicType_ptr dyn_type);

    bool register_dynamic_type_to_factories(
            const TypeSupport& type) const;

    bool check_get_type_request(
            const fastrtps::rtps::SampleIdentity& requestId,
            const fastrtps::types::TypeIdentifier* identifier,
            const fastrtps::types::TypeObject* object,
            fastrtps::types::DynamicType_ptr dyn_type);

    bool check_get_dependencies_request(
            const fastrtps::rtps::SampleIdentity& requestId,
            const fastrtps::types::TypeIdentifierWithSizeSeq& dependencies);

    virtual PublisherImpl* create_publisher_impl(
            const PublisherQos& qos,
            PublisherListener* listener);

    virtual SubscriberImpl* create_subscriber_impl(
            const SubscriberQos& qos,
            SubscriberListener* listener);

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

    IContentFilterFactory* find_content_filter_factory(
            const char* filter_class_name);

    /**
     * Set the DomainParticipantQos checking if the Qos can be updated or not
     *
     * @param to DomainParticipantQos to be updated
     * @param from DomainParticipantQos desired
     * @param first_time Whether the DomainParticipant has been already initialized or not
     *
     * @return true if there has been a changed in one of the attributes that can be updated.
     * false otherwise.
     */
    static bool set_qos(
            DomainParticipantQos& to,
            const DomainParticipantQos& from,
            bool first_time);

    static ReturnCode_t check_qos(
            const DomainParticipantQos& qos);

    static bool can_qos_be_updated(
            const DomainParticipantQos& to,
            const DomainParticipantQos& from);
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_PARTICIPANTIMPL_HPP_ */

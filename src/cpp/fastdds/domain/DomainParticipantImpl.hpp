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

#ifndef FASTDDS_DOMAIN__DOMAINPARTICIPANTIMPL_HPP
#define FASTDDS_DOMAIN__DOMAINPARTICIPANTIMPL_HPP
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <atomic>
#include <condition_variable>
#include <mutex>

#include "fastdds/topic/DDSSQLFilter/DDSFilterFactory.hpp"
#include <fastdds/dds/builtin/topic/ParticipantBuiltinTopicData.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/qos/ReplierQos.hpp>
#include <fastdds/dds/domain/qos/RequesterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include "fastdds/rpc/RequestReplyContentFilterFactory.hpp"
#include <fastdds/dds/rpc/ServiceTypeSupport.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/topic/ContentFilteredTopic.hpp>
#include <fastdds/dds/topic/IContentFilterFactory.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/participant/RTPSParticipantListener.hpp>
#include <fastdds/topic/TopicProxyFactory.hpp>
#include <rtps/reader/StatefulReader.hpp>

namespace eprosima {
namespace fastdds {

namespace rtps {

struct PublicationBuiltinTopicData;
class RTPSParticipant;
struct SubscriptionBuiltinTopicData;

} // namespace rtps

class PublisherAttributes;
class SubscriberAttributes;

namespace dds {
namespace rpc {
class Replier;
class Requester;
class Service;
class ServiceImpl;
} // namespace rpc

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
 * @ingroup FASTDDS_MODULE
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
        auto time_out = std::chrono::time_point<std::chrono::steady_clock>::max();
        if (timeout < std::chrono::seconds::max())
        {
            auto now = std::chrono::steady_clock::now();
            time_out = now + timeout;
        }

        std::unique_lock<std::mutex> lock(mtx_gs_);
        if (!cv_gs_.wait_until(lock, time_out, [this]
                {
                    // Proceed if no callbacks are being executed
                    return !(rtps_listener_.callback_counter_ > 0);
                }))
        {
            return RETCODE_ERROR;
        }

        rtps_listener_.callback_counter_ = (listener == nullptr) ? -1 : 0;
        listener_ = listener;
        return RETCODE_OK;
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
     * @param [out] impl Return a pointer to the created Publisher's implementation.
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
            const fastdds::dds::Duration_t& timeout);

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

    /**
     * Register a service type in this participant.
     * @param service_type The ServiceTypeSupport to register. A copy will be kept by the participant until removed.
     * @param service_type_name The name that will be used to identify the ServiceType.
     */
    ReturnCode_t register_service_type(
            rpc::ServiceTypeSupport service_type,
            const std::string& service_type_name);

    /**
     * Unregister a service type in this participant.
     * @param service_type_name Name of the service type
     */
    ReturnCode_t unregister_service_type(
            const std::string& service_type_name);

    /**
     * Create an enabled RPC service.
     *
     * @param service_name Name of the service.
     * @param service_type_name Type name of the service (Request & reply types)
     * @return Pointer to the created service. nullptr in error case.
     */
    rpc::Service* create_service(
            const std::string& service_name,
            const std::string& service_type_name);

    /**
     * Find a registered RPC service by name
     *
     * @param service_name Name of the service to search for.
     * @return Pointer to the service object if found, nullptr if not found.
     */
    rpc::Service* find_service(
            const std::string& service_name) const;

    /**
     * Delete a registered RPC service.
     *
     * @param service Pointer to the service object to be deleted.
     * @return RETCODE_OK if the service was deleted successfully, RETCODE_ERROR otherwise.
     */
    ReturnCode_t delete_service(
            const rpc::Service* service);

    /**
     * Create a RPC Requester in a given Service.
     *
     * @param service Pointer to a service object where the requester will be created.
     * @param requester_qos QoS of the requester.
     *
     * @return Pointer to the created requester. nullptr in error case.
     */
    rpc::Requester* create_service_requester(
            rpc::Service* service,
            const RequesterQos& requester_qos);

    /**
     * Deletes an existing RPC Requester
     *
     * @param service_name Name of the service where the requester is created.
     * @param requester Pointer to the requester to be deleted.
     * @return RETCODE_OK if the requester was deleted, or an specific error code otherwise.
     */
    ReturnCode_t delete_service_requester(
            const std::string& service_name,
            rpc::Requester* requester);

    /**
     * Create a RPC Replier in a given Service.
     *
     * @param service Pointer to a service object where the Replier will be created.
     * @param requester_qos QoS of the requester.
     *
     * @return Pointer to the created replier. nullptr in error case.
     */
    rpc::Replier* create_service_replier(
            rpc::Service* service,
            const ReplierQos& replier_qos);

    /**
     * Deletes an existing RPC Replier
     *
     * @param service_name Name of the service where the replier is created.
     * @param replier Pointer to the replier to be deleted.
     * @return RETCODE_OK if the replier was deleted, or an specific error code otherwise.
     */
    ReturnCode_t delete_service_replier(
            const std::string& service_name,
            rpc::Replier* replier);

    // TODO create/delete topic

    // TODO Subscriber* get_builtin_subscriber();

    /**
     * @brief Locally ignore a remote domain participant.
     *
     * @param [in] handle Identifier of the remote participant to ignore.
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
     * @param [in] handle Identifier of the remote datawriter to ignore.
     * @return true if correctly ignored. False otherwise.
     */
    bool ignore_publication(
            const InstanceHandle_t& handle);

    /**
     * @brief Locally ignore a remote datareader.
     *
     * @param [in] handle Identifier of the remote datareader to ignore.
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

    ReturnCode_t get_publisher_qos_from_profile(
            const std::string& profile_name,
            PublisherQos& qos) const;

    ReturnCode_t get_publisher_qos_from_xml(
            const std::string& xml,
            PublisherQos& qos) const;

    ReturnCode_t get_publisher_qos_from_xml(
            const std::string& xml,
            PublisherQos& qos,
            const std::string& profile_name ) const;

    ReturnCode_t get_default_publisher_qos_from_xml(
            const std::string& xml,
            PublisherQos& qos) const;

    ReturnCode_t set_default_subscriber_qos(
            const SubscriberQos& qos);

    void reset_default_subscriber_qos();

    const SubscriberQos& get_default_subscriber_qos() const;

    ReturnCode_t get_subscriber_qos_from_profile(
            const std::string& profile_name,
            SubscriberQos& qos) const;

    ReturnCode_t get_subscriber_qos_from_xml(
            const std::string& xml,
            SubscriberQos& qos) const;

    ReturnCode_t get_subscriber_qos_from_xml(
            const std::string& xml,
            SubscriberQos& qos,
            const std::string& profile_name) const;

    ReturnCode_t get_default_subscriber_qos_from_xml(
            const std::string& xml,
            SubscriberQos& qos) const;

    ReturnCode_t set_default_topic_qos(
            const TopicQos& qos);

    void reset_default_topic_qos();

    const TopicQos& get_default_topic_qos() const;

    ReturnCode_t get_topic_qos_from_profile(
            const std::string& profile_name,
            TopicQos& qos) const;

    ReturnCode_t get_topic_qos_from_profile(
            const std::string& profile_name,
            TopicQos& qos,
            std::string& topic_name,
            std::string& topic_data_type) const;

    ReturnCode_t get_topic_qos_from_xml(
            const std::string& xml,
            TopicQos& qos) const;

    ReturnCode_t get_topic_qos_from_xml(
            const std::string& xml,
            TopicQos& qos,
            std::string& topic_name,
            std::string& topic_data_type) const;

    ReturnCode_t get_topic_qos_from_xml(
            const std::string& xml,
            TopicQos& qos,
            const std::string& profile_name) const;

    ReturnCode_t get_topic_qos_from_xml(
            const std::string& xml,
            TopicQos& qos,
            std::string& topic_name,
            std::string& topic_data_type,
            const std::string& profile_name) const;

    ReturnCode_t get_default_topic_qos_from_xml(
            const std::string& xml,
            TopicQos& qos) const;

    ReturnCode_t get_default_topic_qos_from_xml(
            const std::string& xml,
            TopicQos& qos,
            std::string& topic_name,
            std::string& topic_data_type) const;

    ReturnCode_t get_replier_qos_from_profile(
            const std::string& profile_name,
            ReplierQos& qos) const;

    ReturnCode_t get_replier_qos_from_xml(
            const std::string& xml,
            ReplierQos& qos) const;

    ReturnCode_t get_replier_qos_from_xml(
            const std::string& xml,
            ReplierQos& qos,
            const std::string& profile_name) const;

    ReturnCode_t get_default_replier_qos_from_xml(
            const std::string& xml,
            ReplierQos& qos) const;

    ReturnCode_t get_requester_qos_from_profile(
            const std::string& profile_name,
            RequesterQos& qos) const;

    ReturnCode_t get_requester_qos_from_xml(
            const std::string& xml,
            RequesterQos& qos) const;

    ReturnCode_t get_requester_qos_from_xml(
            const std::string& xml,
            RequesterQos& qos,
            const std::string& profile_name) const;

    ReturnCode_t get_default_requester_qos_from_xml(
            const std::string& xml,
            RequesterQos& qos) const;

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
            fastdds::dds::Time_t& current_time) const;

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

    const fastdds::rtps::RTPSParticipant* get_rtps_participant() const
    {
        std::lock_guard<std::mutex> _(mtx_gs_);
        return rtps_participant_;
    }

    fastdds::rtps::RTPSParticipant* get_rtps_participant()
    {
        std::lock_guard<std::mutex> _(mtx_gs_);
        return rtps_participant_;
    }

    const TypeSupport find_type(
            const std::string& type_name) const;

    const rpc::ServiceTypeSupport find_service_type(
            const std::string& service_name) const;

    const InstanceHandle_t& get_instance_handle() const;

    // From here legacy RTPS methods.

    const fastdds::rtps::GUID_t& guid() const;

    std::vector<std::string> get_participant_names() const;

    /**
     * This method can be used when using a StaticEndpointDiscovery mechanism different that the one
     * included in Fast DDS, for example when communicating with other implementations.
     * It indicates the Participant that an Endpoint from the XML has been discovered and
     * should be activated.
     * @param partguid Participant GUID_t.
     * @param userId User defined ID as shown in the XML file.
     * @param kind EndpointKind (WRITER or READER)
     * @return True if correctly found and activated.
     */
    bool new_remote_endpoint_discovered(
            const fastdds::rtps::GUID_t& partguid,
            uint16_t userId,
            fastdds::rtps::EndpointKind_t kind);

    fastdds::rtps::ResourceEvent& get_resource_event() const;

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

    /**
     * @brief Fill a TypeInformationParameter with the type information of a TypeSupport.
     *
     * @param type              TypeSupport to get the type information from.
     * @param type_information  TypeInformationParameter to fill.
     *
     * @return true if the constraints for propagating the type information are met.
     */
    bool fill_type_information(
            const TypeSupport& type,
            xtypes::TypeInformationParameter& type_information);

protected:

    //!Domain id
    DomainId_t domain_id_;

    //!Participant id
    int32_t participant_id_ = -1;

    //!Pre-calculated guid
    fastdds::rtps::GUID_t guid_;

    //!For instance handle creation
    std::atomic<uint32_t> next_instance_id_;

    //!Participant Qos
    DomainParticipantQos qos_;

    //!RTPSParticipant
    fastdds::rtps::RTPSParticipant* rtps_participant_;

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

    //! RPC Service maps
    std::map<std::string, rpc::ServiceTypeSupport> service_types_;
    std::map<std::string, rpc::ServiceImpl*> services_;
    mutable std::mutex mtx_service_types_;
    mutable std::mutex mtx_services_;

    //! RPC Services publisher and subscriber
    std::pair<Subscriber*, SubscriberImpl*> services_subscriber_;
    std::pair<Publisher*, PublisherImpl*> services_publisher_;

    //! RPC Services Reply topic Content Filter Factory
    rpc::RequestReplyContentFilterFactory req_rep_filter_factory_;

    //!Topic map
    std::map<std::string, TopicProxyFactory*> topics_;
    std::map<InstanceHandle_t, Topic*> topics_by_handle_;
    std::map<std::string, std::unique_ptr<ContentFilteredTopic>> filtered_topics_;
    std::map<std::string, IContentFilterFactory*> filter_factories_;
    DDSSQLFilter::DDSFilterFactory dds_sql_filter_factory_;
    mutable std::mutex mtx_topics_;
    std::condition_variable cond_topics_;

    TopicQos default_topic_qos_;

    std::atomic<uint32_t> id_counter_;

    class MyRTPSParticipantListener : public fastdds::rtps::RTPSParticipantListener
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

        void on_participant_discovery(
                fastdds::rtps::RTPSParticipant* participant,
                fastdds::rtps::ParticipantDiscoveryStatus reason,
                const ParticipantBuiltinTopicData& info,
                bool& should_be_ignored) override;

#if HAVE_SECURITY
        void onParticipantAuthentication(
                fastdds::rtps::RTPSParticipant* participant,
                fastdds::rtps::ParticipantAuthenticationInfo&& info) override;
#endif // if HAVE_SECURITY

        void on_reader_discovery(
                fastdds::rtps::RTPSParticipant* participant,
                fastdds::rtps::ReaderDiscoveryStatus reason,
                const fastdds::rtps::SubscriptionBuiltinTopicData& info,
                bool& should_be_ignored) override;

        void on_writer_discovery(
                fastdds::rtps::RTPSParticipant* participant,
                fastdds::rtps::WriterDiscoveryStatus reason,
                const fastdds::rtps::PublicationBuiltinTopicData& info,
                bool& should_be_ignored) override;

        DomainParticipantImpl* participant_;
        int callback_counter_ = 0;

    }
    rtps_listener_;

    void create_instance_handle(
            InstanceHandle_t& handle);

    ReturnCode_t register_dynamic_type(
            DynamicType::_ref_type dyn_type);

    virtual PublisherImpl* create_publisher_impl(
            const PublisherQos& qos,
            PublisherListener* listener);

    virtual SubscriberImpl* create_subscriber_impl(
            const SubscriberQos& qos,
            SubscriberListener* listener);

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

} // namespace dds
} // namespace fastdds
} // namespace eprosima
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif // FASTDDS_DOMAIN__DOMAINPARTICIPANTIMPL_HPP

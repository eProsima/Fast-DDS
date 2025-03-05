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
 * @file SubscriberImpl.hpp
 *
 */

#ifndef _FASTDDS_SUBSCRIBERIMPL_HPP_
#define _FASTDDS_SUBSCRIBERIMPL_HPP_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <map>
#include <mutex>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include <statistics/rtps/monitor-service/interfaces/IStatusQueryable.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSParticipant;
class IPayloadPool;

} //namespace rtps

class TopicAttributes;

namespace dds {

class SubscriberListener;
class DomainParticipant;
class DomainParticipantImpl;
class Subscriber;
class DataReaderImpl;
class TopicDescription;
class TypeSupport;

/**
 * Class SubscriberImpl, contains the actual implementation of the behaviour of the Subscriber.
 *  @ingroup FASTDDS_MODULE
 */
class SubscriberImpl
{
protected:

    friend class DomainParticipantImpl;
    friend class DataReaderImpl;

    /**
     * Create a subscriber, assigning its pointer to the associated writer.
     * Don't use directly, create Subscriber using create_subscriber from Participant.
     */
    SubscriberImpl(
            DomainParticipantImpl* p,
            const SubscriberQos& qos,
            SubscriberListener* listen = nullptr);

public:

    virtual ~SubscriberImpl();

    ReturnCode_t enable();

    const SubscriberQos& get_qos() const;

    ReturnCode_t set_qos(
            const SubscriberQos& qos);

    const SubscriberListener* get_listener() const;

    ReturnCode_t set_listener(
            SubscriberListener* listener);

    DataReader* create_datareader(
            TopicDescription* topic,
            const DataReaderQos& reader_qos,
            DataReaderListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all(),
            std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool = nullptr);

    DataReader* create_datareader_with_profile(
            TopicDescription* topic,
            const std::string& profile_name,
            DataReaderListener* listener,
            const StatusMask& mask = StatusMask::all(),
            std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool = nullptr);

    ReturnCode_t delete_datareader(
            const DataReader* reader);

    DataReader* lookup_datareader(
            const std::string& topic_name) const;

    bool contains_entity(
            const fastdds::rtps::InstanceHandle_t& handle) const;
    /* TODO
       bool begin_access();
     */

    /* TODO
       bool end_access();
     */

    /* TODO When StateKinds are implemented.
       bool get_datareaders(
        std::vector<DataReader*>& readers,
        std::vector<SampleStateKind> sample_states,
        std::vector<ViewStateKind> view_states,
        std::vector<InstanceStateKind> instance_states) const;
     */
    ReturnCode_t get_datareaders(
            std::vector<DataReader*>& readers) const;

    bool has_datareaders() const;

    ReturnCode_t notify_datareaders() const;

    ReturnCode_t set_default_datareader_qos(
            const DataReaderQos& qos);

    void reset_default_datareader_qos();

    const DataReaderQos& get_default_datareader_qos() const;

    DataReaderQos& get_default_datareader_qos();

    ReturnCode_t get_datareader_qos_from_profile(
            const std::string& profile_name,
            DataReaderQos& qos) const;

    ReturnCode_t get_datareader_qos_from_profile(
            const std::string& profile_name,
            DataReaderQos& qos,
            std::string& topic_name) const;

    ReturnCode_t get_datareader_qos_from_xml(
            const std::string& xml,
            DataReaderQos& qos) const;

    ReturnCode_t get_datareader_qos_from_xml(
            const std::string& xml,
            DataReaderQos& qos,
            std::string& topic_name) const;

    ReturnCode_t get_datareader_qos_from_xml(
            const std::string& xml,
            DataReaderQos& qos,
            const std::string& profile_name) const;

    ReturnCode_t get_datareader_qos_from_xml(
            const std::string& xml,
            DataReaderQos& qos,
            std::string& topic_name,
            const std::string& profile_name) const;

    ReturnCode_t get_default_datareader_qos_from_xml(
            const std::string& xml,
            DataReaderQos& qos) const;

    ReturnCode_t get_default_datareader_qos_from_xml(
            const std::string& xml,
            DataReaderQos& qos,
            std::string& topic_name) const;

    ReturnCode_t static copy_from_topic_qos(
            DataReaderQos& reader_qos,
            const TopicQos& topic_qos);

    const DomainParticipant* get_participant() const;

    DomainParticipantImpl* get_participant_impl()
    {
        return participant_;
    }

    const fastdds::rtps::RTPSParticipant* rtps_participant() const
    {
        return rtps_participant_;
    }

    fastdds::rtps::RTPSParticipant* rtps_participant()
    {
        return rtps_participant_;
    }

    const Subscriber* get_subscriber() const
    {
        return user_subscriber_;
    }

    const fastdds::rtps::InstanceHandle_t& get_instance_handle() const;

    //! Remove all listeners in the hierarchy to allow a quiet destruction
    void disable();

    //! Check if any reader uses the given type name
    bool type_in_use(
            const std::string& type_name) const;

    /**
     * Set Qos from another instance
     * @param to Reference to the qos instance to be changed.
     * @param from Reference to the qos instance with the new values.
     * @param first_time Boolean indicating whether is the first time (If not some parameters cannot be set).
     */
    static void set_qos(
            SubscriberQos& to,
            const SubscriberQos& from,
            bool first_time);

    /**
     * Check if the Qos values are compatible between each other.
     * @param qos Reference to the qos instance to check.
     * @return True if correct.
     */
    static ReturnCode_t check_qos(
            const SubscriberQos& qos);

    /**
     * Check if the Qos can be update with the values provided. This method DOES NOT update anything.
     * @param to Reference to the qos instance to be changed.
     * @param from Reference to the qos instance with the new values.
     * @return True if they can be updated.
     */
    static bool can_qos_be_updated(
            const SubscriberQos& to,
            const SubscriberQos& from);

    /**
     * Returns the most appropriate listener to handle the callback for the given status,
     * or nullptr if there is no appropriate listener.
     */
    SubscriberListener* get_listener_for(
            const StatusMask& status);

    ReturnCode_t delete_contained_entities();

    bool can_be_deleted() const;

    /**
     * Check if a given DataReader can be deleted.
     */
    bool can_be_deleted(
            DataReader* reader) const;

#ifdef FASTDDS_STATISTICS
    bool get_monitoring_status(
            statistics::MonitorServiceData& status,
            const fastdds::rtps::GUID_t& entity_guid);
#endif //FASTDDS_STATISTICS

protected:

    //!Participant
    DomainParticipantImpl* participant_;

    SubscriberQos qos_;

    //!Map of Pointer to associated DataReaders. Topic name is the key.
    std::map<std::string, std::vector<DataReaderImpl*>> readers_;

    mutable std::mutex mtx_readers_;

    //!Listener
    SubscriberListener* listener_;

    class SubscriberReaderListener : public DataReaderListener
    {
    public:

        SubscriberReaderListener(
                SubscriberImpl* s)
            : subscriber_(s)
        {
        }

        virtual ~SubscriberReaderListener() override
        {
        }

        void on_data_available(
                DataReader* reader) override;

        void on_subscription_matched(
                DataReader* reader,
                const SubscriptionMatchedStatus& info) override;

        void on_requested_deadline_missed(
                DataReader* reader,
                const RequestedDeadlineMissedStatus& status) override;

        void on_liveliness_changed(
                DataReader* reader,
                const LivelinessChangedStatus& status) override;

        void on_sample_rejected(
                DataReader* reader,
                const SampleRejectedStatus& status) override;

        void on_requested_incompatible_qos(
                DataReader* reader,
                const RequestedIncompatibleQosStatus& status) override;

        void on_sample_lost(
                DataReader* reader,
                const SampleLostStatus& status) override;

        SubscriberImpl* subscriber_;
    }
    subscriber_listener_;

    Subscriber* user_subscriber_;

    //!RTPSParticipant
    fastdds::rtps::RTPSParticipant* rtps_participant_;

    DataReaderQos default_datareader_qos_;

    fastdds::rtps::InstanceHandle_t handle_;

    virtual DataReaderImpl* create_datareader_impl(
            const TypeSupport& type,
            TopicDescription* topic,
            const DataReaderQos& qos,
            DataReaderListener* listener,
            std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool);
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_SUBSCRIBERIMPL_HPP_ */

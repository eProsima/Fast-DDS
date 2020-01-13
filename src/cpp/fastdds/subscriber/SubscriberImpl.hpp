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

#include <fastrtps/attributes/SubscriberAttributes.h>

#include <fastdds/dds/topic/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/topic/qos/DataReaderQos.hpp>
#include <fastrtps/types/TypesBase.h>

#include <mutex>
#include <map>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSParticipant;

} //namespace rtps

class TopicAttributes;

} // namespace fastrtps

namespace fastdds {
namespace dds {

class SubscriberListener;
class DomainParticipant;
class DomainParticipantImpl;
class Subscriber;
class DataReaderImpl;
class Topic;

/**
 * Class SubscriberImpl, contains the actual implementation of the behaviour of the Subscriber.
 *  @ingroup FASTRTPS_MODULE
 */
class SubscriberImpl
{
    friend class DomainParticipantImpl;
    friend class DataReaderImpl;

    /**
     * Create a subscriber, assigning its pointer to the associated writer.
     * Don't use directly, create Subscriber using create_subscriber from Participant.
     */
    SubscriberImpl(
            DomainParticipantImpl* p,
            const SubscriberQos& qos,
            const fastrtps::SubscriberAttributes& attr,
            SubscriberListener* listen = nullptr);

public:

    virtual ~SubscriberImpl();

    const SubscriberQos& get_qos() const;

    ReturnCode_t set_qos(
            const SubscriberQos& qos);

    const SubscriberListener* get_listener() const;

    SubscriberListener* get_listener();

    ReturnCode_t set_listener(
            SubscriberListener* listener);

    DataReader* create_datareader(
            const Topic& topic,
            const fastrtps::TopicAttributes& topic_attr,
            const DataReaderQos& reader_qos,
            DataReaderListener* listener);

    ReturnCode_t delete_datareader(
            DataReader* reader);

    DataReader* lookup_datareader(
            const std::string& topic_name) const;

    bool contains_entity(
            const fastrtps::rtps::InstanceHandle_t& handle) const;
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

    /* TODO
       bool delete_contained_entities();
     */

    ReturnCode_t set_default_datareader_qos(
            const DataReaderQos& qos);

    const DataReaderQos& get_default_datareader_qos() const;

    /* TODO
       bool copy_from_topic_qos(
            DataReaderQos& reader_qos,
            const fastrtps::TopicAttributes& topic_qos) const;
     */

    /**
     * Update the Attributes of the subscriber;
     * @param att Reference to a SubscriberAttributes object to update the parameters;
     * @return True if correctly updated, false if ANY of the updated parameters cannot be updated
     */
    bool set_attributes(
            const fastrtps::SubscriberAttributes& att);

    /**
     * Get the Attributes of the Subscriber.
     * @return Attributes of the Subscriber.
     */
    const fastrtps::SubscriberAttributes& get_attributes() const
    {
        return att_;
    }

    const DomainParticipant* get_participant() const;

    const fastrtps::rtps::RTPSParticipant* rtps_participant() const
    {
        return rtps_participant_;
    }

    fastrtps::rtps::RTPSParticipant* rtps_participant()
    {
        return rtps_participant_;
    }

    const Subscriber* get_subscriber() const
    {
        return user_subscriber_;
    }

    const fastrtps::rtps::InstanceHandle_t& get_instance_handle() const;

    //! Remove all listeners in the hierarchy to allow a quiet destruction
    void disable();

    //! Check if any reader uses the given type name
    bool type_in_use(
            const std::string& type_name) const;

private:

    //!Participant
    DomainParticipantImpl* participant_;

    SubscriberQos qos_;

    //!Attributes of the Subscriber
    fastrtps::SubscriberAttributes att_;

    //!Map of Pointer to associated DataReaders. Topic name is the key.
    std::map<std::string, std::vector<DataReaderImpl*> > readers_;

    mutable std::mutex mtx_readers_;

    //!Listener
    SubscriberListener* listener_;

    class SubscriberReaderListener : public DataReaderListener
    {
public:

        SubscriberReaderListener(
                SubscriberImpl* s)
            : subscriber_(s)
        {}

        virtual ~SubscriberReaderListener() override {}

        void on_data_available(
                DataReader* reader) override;

        void on_subscription_matched(
                DataReader* reader,
                const SubscriptionMatchedStatus& info) override;

        void on_requested_deadline_missed(
                DataReader* reader,
                const fastrtps::RequestedDeadlineMissedStatus& status) override;

        void on_liveliness_changed(
                DataReader* reader,
                const fastrtps::LivelinessChangedStatus& status) override;

        void on_sample_rejected(
                DataReader* reader,
                const fastrtps::SampleRejectedStatus& status) override;

        void on_requested_incompatible_qos(
                DataReader* reader,
                const RequestedIncompatibleQosStatus& status) override;

        void on_sample_lost(
                DataReader* reader,
                const SampleLostStatus& status) override;

        SubscriberImpl* subscriber_;
    } subscriber_listener_;

    Subscriber* user_subscriber_;

    //!RTPSParticipant
    fastrtps::rtps::RTPSParticipant* rtps_participant_;

    DataReaderQos default_datareader_qos_;

    fastrtps::rtps::InstanceHandle_t handle_;

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif
#endif /* _FASTDDS_SUBSCRIBERIMPL_HPP_ */

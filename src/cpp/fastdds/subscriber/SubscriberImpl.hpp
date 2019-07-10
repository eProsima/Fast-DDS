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

#ifndef _FASTDDS_SUBSCRIBERIMPL_H_
#define _FASTDDS_SUBSCRIBERIMPL_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastrtps/attributes/SubscriberAttributes.h>

#include <fastdds/topic/DataReaderListener.hpp>
#include <fastdds/subscriber/qos/SubscriberQos.hpp>

#include <mutex>
#include <map>

namespace eprosima {
namespace fastrtps {
namespace rtps
{
class RTPSReader;
class RTPSParticipant;
}

class TopicAttributes;
class ReaderQos;

} // namespace fastrtps

namespace fastdds {

class SubscriberListener;
class Participant;
class ParticipantImpl;
class Subscriber;

/**
 * Class SubscriberImpl, contains the actual implementation of the behaviour of the Subscriber.
 *  @ingroup FASTRTPS_MODULE
 */
class SubscriberImpl {
    friend class ParticipantImpl;

    /**
     * Create a subscriber, assigning its pointer to the associated writer.
     * Don't use directly, create Subscriber using create_subscriber from Participant.
     */
    SubscriberImpl(
        ParticipantImpl* p,
        const SubscriberQos& qos,
        const fastrtps::SubscriberAttributes& attr,
        SubscriberListener* listen = nullptr);

public:

    virtual ~SubscriberImpl();

    const SubscriberQos& get_qos() const;

    bool set_qos(
            const SubscriberQos& qos);

    const SubscriberListener* get_listener() const;

    bool set_listener(
            SubscriberListener* listener);

    DataReader* create_datareader(
            const fastrtps::TopicAttributes& topic_attr,
            const fastrtps::ReaderQos& reader_qos,
            DataReaderListener* listener);

    bool delete_datareader(
            DataReader* reader);

    DataReader* lookup_datareader(
            const std::string& topic_name) const;

    bool begin_access();

    bool end_access();

    /* TODO When StateKinds are implemented.
    bool get_datareaders(
        std::vector<DataReader*>& readers,
        std::vector<SampleStateKind> sample_states,
        std::vector<ViewStateKind> view_states,
        std::vector<InstanceStateKind> instance_states) const;
    */
    bool get_datareaders(
        std::vector<DataReader*>& readers) const;

    bool notify_datareaders() const;

    bool delete_contained_entities();

    bool set_default_datareader_qos(
            const fastrtps::ReaderQos& qos);

    const fastrtps::ReaderQos& get_default_datareader_qos() const;

    bool copy_from_topic_qos(
            fastrtps::ReaderQos& reader_qos,
            const fastrtps::TopicAttributes& topic_qos) const;

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

    const Participant* get_participant() const;

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

private:

    //!Participant
    ParticipantImpl* participant_;

    SubscriberQos qos_;

    //!Attributes of the Subscriber
    fastrtps::SubscriberAttributes att_;

    //!Map of Pointer to associated DataReaders
    std::map<std::string, DataReader*> readers_;

    mutable std::mutex mtx_readers_;

    //!Listener
    SubscriberListener* listener_;

    class SubscriberReaderListener : public DataReaderListener
    {
    public:
        SubscriberReaderListener(SubscriberImpl* s)
            : subscriber_(s)
        {}

        virtual ~SubscriberReaderListener() override {}

        void on_data_available(
                DataReader* reader) override;

        void on_subscription_matched(
                DataReader* reader,
                fastrtps::rtps::MatchingInfo& info) override;

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
                const fastrtps::RequestedIncompatibleQosStatus& status) override;

        void on_sample_rejected(
                DataReader* reader,
                const fastrtps::SampleLostStatus& status) override;

        SubscriberImpl* subscriber_;
    } subscriber_listener_;

    Subscriber* user_subscriber_;

    //!RTPSParticipant
    fastrtps::rtps::RTPSParticipant* rtps_participant_;

    fastrtps::ReaderQos default_datareader_qos_;

    fastrtps::rtps::InstanceHandle_t handle_;

};


} /* namespace fastdds */
} /* namespace eprosima */
#endif
#endif /* _FASTDDS_SUBSCRIBERIMPL_H_ */
